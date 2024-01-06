/** 
    Copyright (c) 2023, wicked systems
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following
    conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following
      disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
      disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of wicked systems nor the names of its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
    EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/
#include "gateway.h"
#include "reactor.h"
#include <config.h>
#include "event.h"
#include "protocol.h"
#include "mapper.h"
#include "transport.h"
#include "error.h"
#include <convert.h>
#include <cstring>
#include <cmath>
#include <limits>

      constexpr int  s_mtu_min = 32;
      constexpr int  s_mtu_max = 65536;

      constexpr int  s_state_drop = std::numeric_limits<int>::min();    // reject any incoming messages
      constexpr int  s_state_recover = -1;                              // buffer crashed, recover
      constexpr int  s_state_accept = 0;                                // normal state: read in and capture traffic
      constexpr int  s_state_capture_message = 2;                       // normal state, capture message until it completes
      constexpr int  s_state_capture_packet = 3;                        // normal state, capture packet until it completes

namespace emc {

/* brief sanity checks for the global settings
*/
static_assert(packet_head_size > 1, "Packet header size shoud be at least 2");
static_assert(packet_head_size <= 8, "Packet header size shoud be larger than 8 bytes");
static_assert(packet_size_multiplier > 0, "Packet size multiplier should be at least 1");
static_assert(packet_size_max < std::numeric_limits<int>::max(), "Packet size multiplier combined with the size of the packet header exceed integer boundaries"
);

static_assert(s_mtu_min > 0, "Minimum MTU shuld be greater than zero");
static_assert(s_mtu_max >= s_mtu_min, "Maximum MTU shuld be at least equal to the minimum MTU");

static_assert(EOF < 0, "This module requires `char` to be signed, please recompile with \"-fsigned-char\"");
static_assert(message_wait_time < message_drop_time, "message_wait_time should not exceed the message_drop_time");
static_assert(message_trip_time > message_wait_time, "message_trip_time should be greater than message_wait_time");
static_assert(message_ping_time > message_wait_time, "message_ping_time should be greater than message_wait_time");
static_assert(message_ping_time < 
                message_trip_time - message_wait_time - message_wait_time, 
                "message_ping_time should not be greater than the time it takes a ping message to arrive, be"
                "processed and circle back at the destination"
);

      gateway::gateway(unsigned int options) noexcept:
      rawstage(),
      m_recv_data(nullptr),
      m_recv_iter(0),
      m_recv_size(0),
      m_recv_mtu(std::numeric_limits<short int>::max()),
      m_recv_state(s_state_drop),
      m_send_data(nullptr),
      m_send_iter(0),
      m_send_size(0),
      m_send_mtu(mtu_size),
      m_args(),
      p_stage_head(nullptr),
      p_stage_tail(nullptr),
      m_gate_name{0},
      m_gate_info{0},
      m_recv_packet_chid(chid_none),
      m_recv_packet_left(0),
      m_recv_packet_size(0),
      m_send_packet_chid(chid_none),
      m_send_packet_size(0),
      m_gate_ping_time(message_ping_time),
      m_gate_info_time(message_wait_time + message_wait_time),
      m_gate_wait_time(message_wait_time),
      m_gate_drop_time(message_drop_time),
      m_gate_trip_time(message_trip_time),
      m_bt16(options & o_bt16),
      m_bt64(options & o_bt64),
      m_flush_auto(options & o_flush_auto),
      m_flush_sync(false),
      m_ping_ctr(false),
      m_info_ctr(false),
      m_drop_ctr(false),
      m_trip_ctr(false),
      m_ping_await(false),
      m_reserve_min(queue_size_min),
      m_reserve_max(queue_size_max),
      m_stealth_bit(options & o_stealth),
      m_msg_recv(0),
      m_msg_drop(0),
      m_msg_tmit(0),
      m_chr_recv(0),
      m_chr_tmit(0),
      m_mem_size(0),
      m_mem_used(0),
      m_run_time(0.0f),
      m_host_role(false),
      m_user_role(false),
      m_ping_enable(options & o_remote),
      m_stage_count(0),
      m_resume_bit(false),
      m_join_bit(false),
      m_healthy_bit(false)
{
      // reserve memory in the queues
      if(m_reserve_min > 0) {
          m_recv_data = reinterpret_cast<std::uint8_t*>(::malloc(m_reserve_min));
          m_recv_size = m_reserve_min;
          m_send_data = reinterpret_cast<std::uint8_t*>(::malloc(m_reserve_min));
          m_send_size = m_reserve_min;
      }
}

      gateway::~gateway()
{
      emcstage* i_stage = p_stage_tail;
      while(i_stage != nullptr) {
          if(m_resume_bit == true) {
              if(m_join_bit == true) {
                  i_stage->emc_std_drop();
              }
              i_stage->emc_std_suspend(this);
          }
          i_stage->emc_std_detach(this);
          i_stage = i_stage->p_stage_prev;
      }
      if(m_send_data) {
          ::free(m_send_data);
      }
      if(m_recv_data) {
          ::free(m_recv_data);
      }
}

void  gateway::emc_emit(char c) noexcept
{
      if(m_send_packet_chid == chid_none) {
          auto l_emit_ptr = emc_reserve(1);
          if(l_emit_ptr != nullptr) {
              *l_emit_ptr = c;
              m_send_iter += 1;
              // auto-flush enabled: call flush() if the char being stored into the output buffer is EOL
              if(m_flush_auto) {
                  if(*l_emit_ptr == EOL) {
                      flush();
                  }
              }
          }
      }
}

void  gateway::emc_emit(int size, const char* text) noexcept
{
      if(m_send_packet_chid == chid_none) {
          if(size == 0) {
              size = std::strlen(text);
          }
          if(size > 0) {
              auto l_copy_ptr = text;
              auto l_emit_ptr = emc_reserve(size);
              if(l_emit_ptr != nullptr) {
                  std::memcpy(l_emit_ptr, l_copy_ptr, size);
                  m_send_iter += size;
                  if(m_flush_auto) {
                      if(l_emit_ptr[size - 1] == EOL) {
                          flush();
                      }
                  }
              }
          }
      }
}

bool  gateway::emc_prepare_packet(int channel, int size) noexcept
{
      if((m_send_iter == 0) &&
          (m_send_packet_chid == chid_none)) {
          if((channel >= chid_min) &&
              (channel <= chid_max)) {
              if((size >= 0) &&
                  (size < packet_size_max)) {
                  int   l_packet_size = get_round_value(size, packet_size_multiplier);
                  int   l_member_size = packet_head_size - 1;
                  bool  l_reserve_success = emc_reserve(packet_head_size);
                  if(l_reserve_success) {
                      emc_emit(EOF - channel);
                      emc_emit(l_member_size, fmt::x(l_packet_size / packet_size_multiplier, l_member_size));
                      m_send_packet_chid = channel;
                      m_send_packet_size = l_packet_size;
                      m_send_iter        = packet_head_size;
                      return true;
                  }
              }
          }
      }
      return false;
}

bool  gateway::emc_reserve_packet(std::uint8_t*& data, int size) noexcept
{
      if(m_send_packet_chid != chid_none) {
          std::uint8_t* l_packet_base = emc_reserve(m_send_packet_size);
          if(l_packet_base != nullptr) {
              int  l_fill_size;
              int  l_zero_size;
              if(size <= m_send_packet_size) {
                  l_fill_size = size;
              } else
                  l_fill_size = m_send_packet_size;
              if(l_fill_size > 0) {
                  data = l_packet_base;
              }
              l_zero_size = m_send_packet_size - l_fill_size;
              if(l_zero_size > 0) {
                  std::memset(l_packet_base + l_fill_size, 0, l_zero_size);
              }
              m_send_iter += m_send_packet_size;
              return true;
          }
      }
      return false;
}

bool  gateway::emc_fill_packet(std::uint8_t* data, int size) noexcept
{
      if(m_send_packet_chid != chid_none) {
          std::uint8_t* l_packet_base = emc_reserve(m_send_packet_size);
          if(l_packet_base != nullptr) {
              int  l_copy_size;
              int  l_zero_size;
              if(size <= m_send_packet_size) {
                  l_copy_size = size;
              } else
                  l_copy_size = m_send_packet_size;
              if(l_copy_size > 0) {
                  std::memcpy(l_packet_base, data, l_copy_size);
              }
              l_zero_size = m_send_packet_size - l_copy_size;
              if(l_zero_size > 0) {
                  std::memset(l_packet_base + l_copy_size, 0, l_zero_size);
              }
              m_send_iter += m_send_packet_size;
              return true;
          }
      }
      return false;
}

bool  gateway::emc_zero_packet() noexcept
{
      return emc_fill_packet(nullptr, 0);
}

bool  gateway::emc_emit_packet() noexcept
{
      int l_send_rc;
      if(m_send_packet_chid != chid_none) {
          l_send_rc = emc_raw_send(m_send_data, m_send_iter);
          m_send_packet_chid = chid_none;
          m_send_packet_size = 0;
          m_send_iter = 0;
          if(l_send_rc == err_okay) {
              return true;
          }
      }
      return false;
}

bool  gateway::emc_drop_packet() noexcept
{
      if(m_send_packet_chid != chid_none) {
          m_send_packet_chid = chid_none;
          m_send_packet_size = 0;
          m_send_iter = 0;
          return true;
      }
      return false;
}

auto  gateway::emc_reserve(int size) noexcept -> std::uint8_t*
{
      void*  l_reserve_ptr;
      if(size > 0) {
          int  l_reserve_offset = m_send_iter;
          int  l_reserve_size   = l_reserve_offset + size;
          if(l_reserve_size > m_send_size) {
              if(l_reserve_size > m_reserve_max) {
                  return nullptr;
              }
              l_reserve_ptr = ::realloc(m_send_data, l_reserve_size);
              if(l_reserve_ptr == nullptr) {
                  return nullptr;
              }
              m_send_data = reinterpret_cast<std::uint8_t*>(l_reserve_ptr);
              m_send_size = l_reserve_size;
          }
          return m_send_data + m_send_iter;
      }
      if(m_send_data != nullptr) {
          return m_send_data + m_send_iter;
      }
      return nullptr;
}

bool  gateway::emc_resume_at(emcstage* i_stage) noexcept
{
      emcstage* i_stage_prev;
      if(i_stage != nullptr) {
          i_stage_prev = i_stage->p_stage_prev;
          while(i_stage != nullptr) {
              if(i_stage->emc_std_resume(this) == false) {
                  emc_suspend_at(i_stage_prev);
                  return false;
              }
              i_stage_prev = i_stage;
              i_stage      = i_stage->p_stage_next;  
          }
      }
      return true;
}

void  gateway::emc_suspend_at(emcstage* i_stage) noexcept
{
      while(i_stage != nullptr) {
          i_stage->emc_std_suspend(this);
          i_stage = i_stage->p_stage_prev;
      }
}

void  gateway::emc_dispatch_join() noexcept
{
      emcstage* i_stage = p_stage_head;
      while(i_stage != nullptr) {
          i_stage->emc_std_join();
          i_stage = i_stage->p_stage_next;
      }
}

void  gateway::emc_dispatch_drop() noexcept
{
      emcstage* i_stage = p_stage_tail;
      while(i_stage != nullptr) {
          i_stage->emc_std_drop();
          i_stage = i_stage->p_stage_prev;
      }
}

int   gateway::emc_send_ready_response() noexcept
{
      return emc_send_error_response(err_okay);
}

int   gateway::emc_send_info_response() noexcept
{
      const char* l_machine_name = p_owner->get_system_name();
      const char* l_machine_type = p_owner->get_system_type();
      const char* l_architecture_name = os::cpu_name;
      const char* l_architecture_type;

      if((l_machine_name == nullptr) ||
          (l_machine_name[0] == 0)) {
          l_machine_name = emc_machine_name_none;
      }

      if((l_machine_type == nullptr) ||
          (l_machine_type[0] == 0)) {
          l_machine_type = emc_machine_type_default;
      }

      if(os::is_lsb) {
          l_architecture_type = emc_order_le;
      } else
          l_architecture_type = emc_order_be;

      emc_put(
          emc_tag_response, emc_response_info, SPC,
          emc_protocol_name, SPC,
          emc_protocol_version, SPC,
          l_machine_name, SPC,
          l_machine_type, SPC,
          l_architecture_name, '_', l_architecture_type, SPC,
          fmt::X(m_recv_mtu),
          EOL
      );
      return 0;
}

void  gateway::emc_send_info_request() noexcept
{
      emc_put(emc_tag_request, emc_request_info, EOL);
}

void  gateway::emc_send_service_response() noexcept
{
      emcstage* i_stage = p_stage_head;
      // iterate through each stage and list its services by calling onto its `get_layer_name()` member
      emc_put(emc_tag_response, emc_response_service, emc_enable_tag);
      while(i_stage != nullptr) {
          int         l_layer_index = 0;
          const char* p_layer_name;
          int         l_layer_state;
          do {
              p_layer_name = i_stage->get_layer_name(l_layer_index);
              if(p_layer_name != nullptr) {
                  l_layer_state = i_stage->get_layer_state(l_layer_index);
                  if(l_layer_state == emc::emcstage::layer_state_enabled) {
                      emc_put(SPC);
                      if(p_layer_name[0]) {
                          emc_put(p_layer_name);
                      } else
                          emc_put("?");
                  }
              }
              ++l_layer_index;
          }
          while(p_layer_name != nullptr);
          i_stage = i_stage->p_stage_next;
      }
      // finish the services line
      emc_put(EOL);
}

void  gateway::emc_send_support_response() noexcept
{
      emcstage* i_stage = p_stage_head;
      // iterate through each stage and ask it to list its support descriptors
      while(i_stage != nullptr) {
          i_stage->describe();
          i_stage = i_stage->p_stage_next;
      }
}

void  gateway::emc_send_ping_request() noexcept
{
      if(m_ping_await == false) {
          // calculate time to send out (in 16ths of a millisecond)
          float l_exec_time = fmodf(m_run_time, 86400.00f);
          int   l_ping_time = l_exec_time / 0.0010625f;
          if(l_ping_time > 0) {
              emc_put(emc_tag_request, emc_request_ping, SPC, fmt::X(l_ping_time), EOL);
              m_ping_await = true;
          }
      }
}

int   gateway::emc_send_pong_response(const char* word) noexcept
{
      if(word != nullptr) {
          emc_put(emc_tag_response, emc_response_pong, SPC, word, EOL);
      } else
          emc_put(emc_tag_response, emc_response_pong, EOL);
      return err_okay;
}

int   gateway::emc_send_bye_response() noexcept
{
      return emc_send_ready_response();
}

int   gateway::emc_send_sync_response() noexcept
{
      emc_send_info_response();
      emc_send_service_response();
      emc_send_support_response();
      return err_okay;
}

int   gateway::emc_send_help_response() noexcept
{
      return err_okay;
}

void  gateway::emc_send_raw(const char* message, int length) noexcept
{
      emc_emit(length, message);
}

int   gateway::emc_send_error_response(int rc, const char* message, ...) noexcept
{
      const char* l_message_0 = nullptr;
      const char* l_message_1 = message;
      bool        l_message_insert_separator = false;
      bool        l_message_insert_terminator = false;
      switch(rc) {
          case err_okay:
              l_message_0 = msg_ready;
              break;
          case err_parse:
              l_message_0 = msg_parse;
              break;
          case err_refuse:
              l_message_0 = msg_refuse;
              break;
          case err_bad_request:
              l_message_0 = msg_bad_request;
              break;
          case err_no_request:
              l_message_0 = msg_no_request;
              break;
          default:
              break;
      }
      emc_put(emc_tag_response, fmt::X(rc, 2));
      if(l_message_0) {
          if(l_message_0[0]) {
              emc_put(SPC, l_message_0);
              l_message_insert_separator = true;
              l_message_insert_terminator = true;
          }
      }
      if(l_message_1) {
          if(l_message_1[0]) {
              if(l_message_insert_separator) {
                  emc_put(':');
              }
              emc_put(SPC);
              emc_put(l_message_1);
              l_message_insert_terminator = true;
          }
      }
      if(l_message_insert_terminator) {
          emc_put('.');
      }
      emc_put(EOL);
      return rc;
}

void  gateway::emc_gate_dispatch_connect(const char* name, const char* info, int mtu) noexcept
{
      emcstage* i_stage = p_stage_head;
      while(i_stage != nullptr) {
          i_stage->emc_std_connect(name, info, mtu);
          i_stage = i_stage->p_stage_next;
      }
}

void  gateway::emc_gate_dispatch_message(const char* message, int length) noexcept
{
      emcstage* i_stage = p_stage_head;
      while(i_stage != nullptr) {
          i_stage->emc_std_process_message(message, length);
          i_stage = i_stage->p_stage_next;
      }
}

int   gateway::emc_gate_forward_request(int argc, const sys::argv& argv) noexcept
{
      if(p_stage_head != nullptr) {
          return p_stage_head->emc_std_process_request(argc, argv);
      }
      return err_no_request;
}

int   gateway::emc_gate_forward_response(int argc, const sys::argv& argv) noexcept
{
      if(p_stage_head != nullptr) {
          return p_stage_head->emc_std_process_response(argc, argv);
      }
      return err_no_response;
}

void  gateway::emc_gate_dispatch_comment(const char* message, int length) noexcept
{
      emcstage* i_stage = p_stage_head;
      while(i_stage != nullptr) {
          i_stage->emc_std_process_comment(message, length);
          i_stage = i_stage->p_stage_next;
      }
}

int   gateway::emc_gate_forward_packet(int channel, int size, std::uint8_t* data) noexcept
{
      if(p_stage_head != nullptr) {
          return p_stage_head->emc_std_process_packet(channel, size, data);
      }
      return err_okay;
}

int   gateway::emc_std_return_message(const char* message, int length) noexcept
{
      emc_emit(length, message);
      return err_okay;
}

int   gateway::emc_std_return_packet(int channel, int size, std::uint8_t* data) noexcept
{
      if((channel >= chid_min) &&
          (channel <= chid_max)) {
          // TODO
          return err_okay;
      }
      return err_fail;
}

void  gateway::emc_gate_dispatch_disconnect() noexcept
{
      emcstage* i_stage = p_stage_head;
      while(i_stage != nullptr) {
          i_stage->emc_std_disconnect();
          i_stage = i_stage->p_stage_next;
      }
}

int   gateway::emc_std_event(int id, void* data) noexcept
{
      return emc_raw_event(id, data);
}

void  gateway::emc_gate_drop() noexcept
{
      // drop the current message in the receive queue, if present
      if(m_recv_iter > 0) {
          m_recv_iter = 0;
          m_recv_state = s_state_recover;
      }
}

void  gateway::emc_gate_trip() noexcept
{
      // mark the connection as unhealthy, disable timers and refuse to process further messages
      if(m_user_role) {
          emc_raw_event(ev_drop, nullptr);
      }
}

auto  gateway::emc_get_gate_name() const noexcept -> const char*
{
      return m_gate_name;
}

auto  gateway::emc_get_gate_info() const noexcept -> const char*
{
      return m_gate_info;
}

int   gateway::emc_get_send_mtu() const noexcept
{
      return m_send_mtu;
}

bool  gateway::emc_set_send_mtu(int mtu) noexcept
{
      if(mtu >= s_mtu_min) {
          if(mtu <= s_mtu_max) {
              m_send_mtu = mtu;
              return true;
          }
      }
      return false;
}

int   gateway::emc_raw_feed_request(char* message, int length) noexcept
{
      int         l_rc    = err_parse;
      const char* l_error = nullptr;
      // dispatch the raw message to whatever listeners there may be waiting for it
      emc_gate_dispatch_message(message, length);
      // load the received request string onto the command pre-parser
      if(true) {
          int         l_argc  = m_args.load(message + 1);
          auto&       l_argv  = m_args;
          if(l_argc > 0) {
              // give `emc_process_request()` priority in handling the request
              l_rc = emc_process_request(l_argc, l_argv);
              // ...or otherwise handle the protocol-mandated requests here
              if(l_rc == err_no_request) {
                  if(l_argv[0].has_size(1)) {
                      switch(l_argv[0][0]) {
                          case emc_request_info:
                              if(l_argc == 1) {
                                  l_rc = emc_send_info_response();
                              } else
                                  l_rc = err_parse;
                              break;
                          case emc_request_ping:
                              if(l_argc == 2) {
                                  l_rc = emc_send_pong_response(l_argv[1].get_text());
                              } else
                              if(l_argc == 1) {
                                  l_rc = emc_send_pong_response(nullptr);
                              } else
                                  l_rc = err_parse;
                              break;
                          case emc_request_bye:
                              l_rc = emc_raw_event(ev_close_request, nullptr);
                              break;
                      }
                  }
                  // forward the request to the next stage
                  if(l_rc == err_no_request) {
                      l_rc = emc_gate_forward_request(l_argc, l_argv);
                  }
              }
          }
          // ...or otherwise still - send an error
          if(l_rc != err_okay) {
              l_rc = emc_send_error_response(l_rc, l_error);
          }
      }
      return l_rc;
}

void  gateway::emc_raw_feed_help_request(char* message, int length) noexcept
{
      emc_gate_dispatch_message(message, length);
      emc_send_help_response();
}

void  gateway::emc_raw_feed_sync_request(char* message, int length) noexcept
{
      emc_gate_dispatch_message(message, length);
      emc_send_sync_response();
}

void  gateway::emc_raw_feed_response(char* message, int length) noexcept
{
      // dispatch the raw response to whatever listeners there may be expecting for it
      emc_gate_dispatch_message(message, length);
      // load the received response string onto the command pre-parser
      if(true) {
          int   l_rc   = err_parse;
          int   l_argc = m_args.load(message + 1);
          auto& l_argv = m_args;
          if(l_argc > 0) {
              // give `emc_process_response()` priority in handling the response
              l_rc = emc_process_response(l_argc, l_argv);
              // ...or otherwise handle standard responses here
              if(l_rc == err_no_response) {
                  int  l_tag = m_args[0][0];
                  switch(l_tag) {
                      case emc_response_info: {
                          if(m_args[1].has_text(emc_protocol_name)) {
                              if(m_args[2].has_text(emc_protocol_version, 0, 3)) {
                                  const char* l_name;
                                  const char* l_info;
                                  int         l_mtu  = emc_get_send_mtu();
                                  if(m_args[3].has_text()) {
                                      if(m_args[3].get_size() <= emc_name_size) {
                                          l_name = m_args[3].get_text();
                                      } else
                                          break;
                                  } else
                                      break;
                                  if(m_args[4].has_text()) {
                                      if(m_args[4].get_size() <= emc_info_size) {
                                          l_info = m_args[4].get_text();
                                      } else
                                          break;
                                  } else
                                      break;
                                  if(m_args[6].has_text()) {
                                      l_mtu = m_args[6].get_hex_int();
                                      std::strncpy(m_gate_name, l_name, emc_name_size);
                                      std::strncpy(m_gate_info, l_info, emc_info_size);
                                      if((l_mtu >= s_mtu_min) &&
                                          (l_mtu <= s_mtu_max)) {
                                          emc_set_send_mtu(l_mtu);
                                      }
                                      m_info_ctr.suspend();
                                      m_trip_ctr.reset();
                                      if(m_healthy_bit == false) {
                                          emc_gate_dispatch_connect(m_gate_name, m_gate_info, l_mtu);
                                          m_healthy_bit = true;
                                      }
                                      l_rc = err_okay;
                                  } else
                                      break;
                              }
                          }
                      }
                          break;
                      case emc_response_service:
                          m_trip_ctr.reset();
                          l_rc = err_okay;
                          break;
                      case emc_response_pong:
                          m_trip_ctr.reset();
                          m_ping_await = false;
                          l_rc = err_okay;
                          break;
                      case '0':
                      case '1':
                      case '2':
                      case '3':
                      case '4':
                      case '5':
                      case '6':
                      case '7':
                      case '8':
                      case '9':
                      case 'A':
                      case 'B':
                      case 'C':
                      case 'D':
                      case 'E':
                      case 'F':
                          m_trip_ctr.reset();
                          l_rc = err_okay;
                          break;
                      case emc_response_bye:
                          m_trip_ctr.reset();
                          l_rc = emc_raw_event(ev_close_request, nullptr);
                          break;
                      default:
                          break;
                  }
              }
          }
          // ...or otherwise still - send an error
          if(l_rc != err_okay) {
              // 'no response' is a silent error
              if(l_rc != err_no_response) {
                  emc_send_error_response(l_rc);
              }
          }
      }
}

void  gateway::emc_raw_feed_comment(char* message, int length) noexcept
{
      emc_gate_dispatch_comment(message, length);
}

void  gateway::emc_raw_feed_packet(int channel, int size, std::uint8_t* data) noexcept
{
      emc_gate_forward_packet(channel, size, data);
}

void  gateway::emc_raw_attach(reactor*) noexcept
{
}

bool  gateway::emc_raw_resume(reactor* reactor) noexcept
{
      if((m_user_role == false) &&
          (m_host_role == false)) {
          m_join_bit = false;
          m_healthy_bit = false;
          m_msg_recv = 0;
          m_msg_drop = 0;
          m_msg_tmit = 0;
          if(reactor->has_role(reactor::role::user)) {
              m_user_role = true;
              m_resume_bit = emc_resume_at(p_stage_head);
              if(m_resume_bit) {
                  m_recv_state = s_state_accept;
              }
          } else
          if(reactor->has_role(reactor::role::host) ||
              reactor->has_role(reactor::role::proxy)) {
              m_host_role = reactor->has_role(reactor::role::host);
              m_resume_bit = emc_resume_at(p_stage_head);
              if(m_resume_bit) {
                  m_join_bit = true;
                  m_healthy_bit = true;
                  m_recv_state = s_state_accept;
                  if(m_host_role) {
                      emc_send_sync_response();
                  }
              }
          }
      }
      return m_resume_bit;
}

void  gateway::emc_raw_join() noexcept
{
      m_ping_ctr.reset();
      m_info_ctr.reset();
      m_drop_ctr.reset();
      m_drop_ctr.resume();
      m_trip_ctr.reset();
      if(m_user_role) {
          if(m_ping_enable) {
              if(p_owner->get_ring_flags() <= emc::reactor::emi_ring_network) {
                  m_ping_ctr.resume();
              }
          }
          m_trip_ctr.resume(m_ping_enable);
          m_info_ctr.resume();
          m_join_bit = true;
          emc_dispatch_join();
      }
      m_run_time = 0.0f;
}

int   gateway::emc_raw_feed(std::uint8_t* data, int size) noexcept
{
      char*  p_feed;
      char*  p_last;
      int    l_read_size;
      int    l_feed_iter;
      int    l_send_rc;
      int    l_feed_rc = err_okay;
      if(m_recv_state != s_state_drop) {
          l_feed_iter = 0;
          while(l_feed_iter < size) {
              p_feed      = reinterpret_cast<char*>(data + l_feed_iter);
              p_last      = reinterpret_cast<char*>(data + size);
              l_read_size = 0;
              if(m_recv_state >= s_state_accept) {
                  if(m_recv_state == s_state_accept) {
                      m_recv_iter = 0;
                      m_recv_packet_chid = chid_none;
                      m_recv_packet_left = 0;
                      m_recv_packet_size = 0;
                      // check the message type
                      if((*p_feed > EOF) &&
                          (*p_feed < ASCII_MAX)) {
                          // regular message, set state to '_capture_message'
                          m_recv_state = s_state_capture_message;
                      } else
                      if(*p_feed < EOF) {
                          // packet, set state to '_capture_packet'
                          m_recv_state = s_state_capture_packet;
                      } else
                      if(*p_feed == EOF) {
                          // caugth EOF, ignore in this context
                      } else
                      if(*p_feed == ASCII_MAX) {
                          // caugth \x7f, ignore for now, decide what TODO later
                      }
                  }
                  if(m_recv_state == s_state_capture_message) {
                      bool  l_commit = false;
                      bool  l_reject = false;
                      // run through the inbound data and copy it into the receive buffer, until the first RET and/or EOL
                      while(p_feed < p_last) {
                          l_read_size++;
                          if(m_recv_iter == m_recv_size) {
                              void* l_recv_data;
                              int   l_recv_reserve;
                              if(m_recv_iter >= m_reserve_max) {
                                  l_reject  = true;
                                  l_feed_rc = err_fail;
                                  break;
                              }
                              l_recv_reserve = m_recv_size + size;
                              if(l_recv_reserve > m_reserve_max) {
                                  l_recv_reserve = m_reserve_max;
                              }
                              l_recv_data = realloc(m_recv_data, l_recv_reserve);
                              if(l_recv_data == nullptr) {
                                  l_reject  = true;
                                  l_feed_rc = err_fail;
                                  break;
                              }
                              m_recv_data = reinterpret_cast<std::uint8_t*>(l_recv_data);
                              m_recv_size = l_recv_reserve;
                          }
                          if(*p_feed == NUL) {
                              m_recv_data[m_recv_iter++] = NUL;
                              l_commit = true;
                              break;
                          } else
                          if(*p_feed == RET) {
                              m_recv_data[m_recv_iter++] = NUL;
                              l_commit = true;
                              break;
                          } else
                          if(*p_feed == EOL) {
                              if(m_recv_iter > 0) {
                                  if(m_recv_data[m_recv_iter - 1] == RET) {
                                      m_recv_data[m_recv_iter - 1] = NUL;
                                  } else
                                      m_recv_data[m_recv_iter++] = NUL;
                              } else
                                  m_recv_data[m_recv_iter++] = NUL;
                              l_commit = true;
                              break;
                          }
                          m_recv_data[m_recv_iter++] = *p_feed;
                          p_feed++;
                      }
                      // check the state set by the feed loop
                      if(l_reject) {
                          if((*p_feed == NUL) ||
                              (*p_feed == EOL)) {
                              m_msg_drop++;
                              m_recv_state = s_state_accept;
                          } else
                              m_recv_state = s_state_recover;
                          m_recv_iter = 0;
                      } else
                      if(l_commit) {
                          char* p_recv        = reinterpret_cast<char*>(m_recv_data);
                          int   l_recv_length = m_recv_iter;
                          if(m_host_role == true) {
                              // capture requests, if gateway is running as server
                              if(p_recv[0] == emc_tag_request) {
                                  emc_raw_feed_request(p_recv, l_recv_length);
                              } else
                              if((p_recv[0] == emc_tag_help) && (p_recv[1] == NUL)) {
                                  emc_raw_feed_help_request(p_recv, l_recv_length);
                              } else
                              if((p_recv[0] == emc_tag_sync) && (p_recv[1] == NUL)) {
                                  emc_raw_feed_sync_request(p_recv, l_recv_length);
                              } else
                                  emc_raw_feed_comment(p_recv, l_recv_length);
                          } else
                          if(m_user_role == true) {
                              // capture responses, if waiting for any
                              if(p_recv[0] == emc_tag_response) {
                                  emc_raw_feed_response(p_recv, l_recv_length);
                              } else
                                  emc_raw_feed_comment(p_recv, l_recv_length);
                          }
                          m_recv_state = s_state_accept;
                          m_msg_recv++;
                      }
                      // clear the message drop timer 
                      m_drop_ctr.reset();
                  } else
                  if(m_recv_state == s_state_capture_packet) {
                      bool l_reject = false;
                      int  l_feed_size = size - l_feed_iter;
                      // run through the inbound data and copy it into the receive buffer, until the established packet
                      // size is achieved
                      // if the size of the packet is not yet known (i.e. not all packet header bytes received yet) then read
                      // in just the initial header bytes
                      if(m_recv_packet_chid != chid_none) {
                          if(l_feed_size >= m_recv_packet_left) {
                              l_read_size = m_recv_packet_left;
                          } else
                              l_read_size = l_feed_size;
                      } else
                      if(m_recv_iter + l_feed_size > packet_head_size) {
                          l_read_size = packet_head_size - m_recv_iter;
                      } else
                          l_read_size = l_feed_size;
                      // check if we have enough room to store the currently incomplete inbound message into our local buffer
                      int  l_recv_reserve = m_recv_iter + l_read_size;
                      if(l_recv_reserve >= m_reserve_max) {
                          l_reject  = true;
                          l_feed_rc = err_fail;
                      }
                      if(l_feed_rc == err_okay) {
                          void* l_recv_data;
                          if(l_recv_reserve > m_recv_size) {
                                l_recv_data = realloc(m_recv_data, l_recv_reserve);
                                if(l_recv_data == nullptr) {
                                    l_reject  = true;
                                    l_feed_rc = err_fail;
                                }
                                if(l_feed_rc == err_okay) {
                                    m_recv_data = reinterpret_cast<std::uint8_t*>(l_recv_data);
                                    m_recv_size = l_recv_reserve;
                                }
                          }
                      }
                      // copy the message into our local buffer
                      if(l_feed_rc == err_okay) {
                          std::memcpy(m_recv_data + m_recv_iter, p_feed, l_read_size);
                          m_recv_iter += l_read_size;
                          // load the packet header if not previously completed
                          if(m_recv_packet_chid == chid_none) {
                              if(m_recv_iter >= packet_head_size) {
                                  int   l_encoded_chid;
                                  int   l_encoded_size;
                                  char* p_packet_header = reinterpret_cast<char*>(m_recv_data);
                                  if(convert<char*>::get_hex_value(l_encoded_size, p_packet_header + 1, packet_head_size - 1)) {
                                      m_recv_packet_size = l_encoded_size * packet_size_multiplier;
                                      m_recv_packet_left = m_recv_packet_size;
                                  }
                                  l_encoded_chid = EOF - p_packet_header[0];
                                  if((l_encoded_chid >= chid_min) &&
                                      (l_encoded_chid <= chid_max)) {
                                      m_recv_packet_chid = l_encoded_chid;
                                  }
                                  else {
                                      l_reject  = true;
                                      l_feed_rc = err_fail;
                                  }
                              }
                          } else
                          if(m_recv_packet_chid != chid_none) {
                              m_recv_packet_left -= l_read_size;
                          }
                      }
                      // drop the message if the reject flag has been set
                      if(l_reject) {
                          m_recv_packet_size -= m_recv_iter;
                          if(m_recv_packet_size == 0) {
                              m_msg_drop++;
                              m_recv_state = s_state_accept;
                          } else
                              m_recv_state = s_state_recover;
                      } else
                      if(m_recv_iter == m_recv_packet_size + packet_head_size) {
                          // dispatch the message if it's completed
                          std::uint8_t* p_recv        = m_recv_data + packet_head_size;
                          int           l_recv_length = m_recv_iter - packet_head_size;
                          if(m_host_role == true) {
                              emc_raw_feed_packet(m_recv_packet_chid, l_recv_length, p_recv);
                          } else
                          if(m_user_role == true) {
                              // emc_raw_return_packet(m_recv_packet_chid, l_recv_length, p_recv);
                          }
                          m_recv_state = s_state_accept;
                          m_msg_recv++;
                      }
                      // clear the message drop timer 
                      m_drop_ctr.reset();
                  }
              }
              if(m_recv_state == s_state_recover) {
                  // recover from an error in message acquisition or from an unknown state
                  if(m_recv_packet_left > 0) {
                      // skip until the end of the packet and restore the `accept` state once reached
                      if(m_recv_packet_left <= size) {
                          l_read_size  = m_recv_packet_left;
                          m_recv_state = s_state_accept;
                          m_msg_drop++;
                      } else
                          l_read_size = size;
                      m_recv_packet_left -= l_read_size;
                  } else
                  if(true) {
                      // run through the inbound data and copy it into the receive buffer, until the first end of line;
                      while(p_feed < p_last) {
                          l_read_size++;
                          if((*p_feed == NUL) || (*p_feed == RET) || (*p_feed == EOL)) {
                              m_recv_state = s_state_accept;
                              m_msg_drop++;
                              break;
                          }
                          p_feed++;
                      }
                  }
                  // clear the message drop timer 
                  m_drop_ctr.reset();
              }
              l_feed_iter += l_read_size;
          }
      }
      // update the transfer statistics
      m_chr_recv += size;
      // encountered activity, clear the ping and trip timers
      m_ping_ctr.reset();
      m_trip_ctr.reset();
      // forward the event further down the pipeline
      l_send_rc = rawstage::emc_raw_feed(data, size);
      if(l_send_rc != err_okay) {
          return l_send_rc;
      }
      return l_feed_rc;
}

int   gateway::emc_raw_send(std::uint8_t* data, int size) noexcept
{
      return rawstage::emc_raw_send(data, size);
}

void  gateway::emc_raw_drop() noexcept
{
      // external event implies that the connection with the endpoint has been lost
      if(m_join_bit) {
          if(m_user_role) {
              if(m_healthy_bit) {
                  emc_gate_dispatch_disconnect();
                  m_healthy_bit = false;
              }
              m_user_role = false;
          }
          emc_dispatch_drop();
          m_host_role = false;
          m_recv_state = s_state_drop;
          m_join_bit = false;
      }
      // drop the messages inbound and outbound messages from their respective queues
      m_recv_iter = 0;
      m_send_iter = 0;
      // suspend timers
      m_ping_ctr.suspend();
      m_info_ctr.suspend();
      m_drop_ctr.suspend();
      m_trip_ctr.suspend();
      m_ping_await = false;
}

void  gateway::emc_raw_suspend(emc::reactor*) noexcept
{
      // external event wants us to suspend operation
      if(m_resume_bit) {
          if(m_join_bit) {
              if(m_healthy_bit) {
                  emc_gate_dispatch_disconnect();
                  m_healthy_bit = false;
              }
              emc_dispatch_drop();
              m_join_bit = false;
          }
          // drop the messages inbound and outbound messages from their respective queues
          m_recv_iter = 0;
          m_send_iter = 0;
          // suspend timers
          m_ping_ctr.suspend();
          m_info_ctr.suspend();
          m_drop_ctr.suspend();
          m_trip_ctr.suspend();
          emc_suspend_at(p_stage_tail);
          m_user_role = false;
          m_host_role = false;
          m_recv_state  = s_state_drop;
          m_gate_name[0] = 0;
          m_gate_info[0] = 0;
          m_ping_await = false;
          m_resume_bit = false;
      }
}

int   gateway::emc_raw_event(int id, void* stage) noexcept
{
      return rawstage::emc_raw_event(id, stage);
}

void  gateway::emc_raw_detach(reactor*) noexcept
{
}

void  gateway::emc_raw_sync(float dt) noexcept
{
      m_ping_ctr.update(dt);
      if(m_ping_ctr.test(m_gate_ping_time)) {
          emc_send_ping_request();
          m_ping_ctr.reset();
      }
      m_info_ctr.update(dt);
      if(m_info_ctr.test(m_gate_info_time)) {
          emc_send_info_request();
          m_info_ctr.reset();
      }
      // update the drop timer and check if it is time to drop the incomplete message in the queue, if present
      m_drop_ctr.update(dt);
      if(m_drop_ctr.test(m_gate_drop_time)) {
          emc_gate_drop();
          m_drop_ctr.reset();
      }
      // update the trip timer
      m_trip_ctr.update(dt);
      if(m_trip_ctr.test(m_gate_trip_time)) {
          emc_gate_trip();
          m_trip_ctr.reset();
      }
      // dispatch the sync() signal to the pipeline
      emcstage* i_stage = p_stage_head;
      while(i_stage != nullptr) {
          i_stage->emc_std_sync(dt);
          i_stage = i_stage->p_stage_next;
      }
      // update the uptime counter
      if(m_join_bit) {
          m_run_time += dt;
      }
}

int   gateway::emc_process_request(int argc, const sys::argv& argv) noexcept
{
      return err_no_request;
}

int   gateway::emc_process_response(int argc, const sys::argv& argv) noexcept
{
      return err_no_response;
}

bool  gateway::attach(emcstage* stage) noexcept
{
      if(stage != nullptr) {
          if(stage->p_owner == nullptr) {
              // prepare the stage
              stage->p_owner = this;
              stage->emc_std_attach(this);
              if(m_resume_bit) {
                  if(stage->emc_std_resume(this) == false) {
                      emc_raw_event(emc::ev_hard_fault, this);
                      stage->p_owner = nullptr;
                      return false;
                  }
                  if(m_join_bit) {
                      stage->emc_std_join();
                  }
              }
              // link the new stage into the gateway list
              stage->p_stage_prev = p_stage_tail;
              stage->p_stage_next = nullptr;
              if(p_stage_tail != nullptr) {
                  p_stage_tail->p_stage_next = stage;
              } else
                  p_stage_head = stage;
              p_stage_tail = stage;
              // bring the stage up
              m_stage_count++;
          }
          return stage->p_owner == this;
      }
      return false;
}

bool  gateway::detach(emcstage* stage) noexcept
{
      if(stage != nullptr) {
          if(stage->p_owner == this) {
              if(m_resume_bit) {
                  if(m_join_bit) {
                      stage->emc_std_drop();
                  }
                  stage->emc_std_suspend(this);
              }
              stage->emc_std_detach(this);
              if(stage->p_stage_prev != nullptr) {
                  stage->p_stage_prev->p_stage_next = stage->p_stage_next;
              } else
                  p_stage_head = stage->p_stage_next;
              if(stage->p_stage_next != nullptr) {
                  stage->p_stage_next->p_stage_prev = stage->p_stage_prev;
              } else
                  p_stage_tail = stage->p_stage_prev;
              stage->p_stage_next = nullptr;
              stage->p_stage_prev = nullptr;
              stage->p_owner = nullptr;
              m_stage_count--;
          }
          return stage->p_owner == nullptr;
      }
      return true;
}

bool  gateway::send_line(const char* message, std::size_t length) noexcept
{
      if(m_resume_bit) {
          if(m_healthy_bit) {
              if(m_user_role) {
                  std::size_t l_length_max = static_cast<unsigned int>(m_send_mtu);
                  if(length <= l_length_max) {
                      emc_emit(length, message);
                      emc_emit(EOL);
                      return true;
                  }
              }
          }
      }
      return false;
}

bool  gateway::send_packet_raw(int channel, std::uint8_t* data, std::size_t size) noexcept
{
      if(m_resume_bit) {
          if(m_healthy_bit) {
              if(m_user_role) {
                  if(emc_prepare_packet(channel, size)) {
                      int  l_packet_size    = m_send_packet_size;
                      int  l_packet_padding = l_packet_size - size;
                      if((l_packet_size > m_send_size) ||
                          (l_packet_size > global::cache_large_max)) {
                          // strategy for large packets: send in 3 chunks: header, data and padding
                          if(emc_emit_packet() == false) {
                              return false;
                          }
                          if(emc_raw_send(data, size) != emc::err_okay) {
                              return false;
                          }
                          // if necessary, pad the buffer and send the padding out; use the send buffer, since it's already
                          // secured by `emc_prepare_packet()`
                          if(l_packet_padding > 0) {
                              if(emc_reserve(l_packet_padding) == nullptr) {
                                  return false;
                              }
                              std::memset(m_send_data, 0, l_packet_padding);
                              if(emc_raw_send(m_send_data, l_packet_padding) != err_okay) {
                                  return false;
                              }
                          }
                      } else
                      if(l_packet_size > 0) {
                          // strategy for smaller packets: copy into the internal buffer and send in one shot
                          if(emc_fill_packet(data, size) == false) {
                              return false;
                          }
                          if(emc_emit_packet() == false) {
                              return false;
                          }   
                      }
                      return true;
                  }
              }
          }
      }
      return false;
}

bool  gateway::send_packet_base16(int channel, std::uint8_t* data, std::size_t size) noexcept
{
      if(m_resume_bit) {
          if(m_healthy_bit) {
              if(m_user_role) {
                  if(size < std::numeric_limits<int>::max() / 2) {
                      std::uint8_t* l_packet_base;
                      int           l_packet_size = static_cast<int>(size) * 2;
                      if(emc_prepare_packet(channel, l_packet_size)) {
                          if(emc_reserve_packet(l_packet_base, l_packet_size)) {
                              emc::transport::base16_encode(l_packet_base, data, m_send_packet_size);
                              if(emc_emit_packet()) {
                                  return true;
                              }
                          }
                      }
                  }
              }
          }
      }
      return false;
}

bool  gateway::send_packet_base64(int channel, std::uint8_t* data, std::size_t size) noexcept
{
      if(m_resume_bit) {
          if(m_healthy_bit) {
              if(m_user_role) {
                  if(size < std::numeric_limits<int>::max() / 4) {
                      std::uint8_t* l_packet_base;
                      int           l_packet_size = get_round_value(static_cast<int>(size) * 4 / 3, 4);
                      if(emc_prepare_packet(channel, l_packet_size)) {
                          if(emc_reserve_packet(l_packet_base, l_packet_size)) {
                              emc::transport::base64_encode(l_packet_base, data, m_send_packet_size);
                              if(emc_emit_packet()) {
                                  return true;
                              }
                          }
                      }
                  }
              }
          }
      }
      return false;
}

bool  gateway::set_drop_time(float value) noexcept
{
      if((value > 0.0f) &&
          (value < 300.0f)) {
          m_gate_drop_time = value;
          return true;
      }
      return false;
}

bool  gateway::set_trip_time(float value) noexcept
{
      if((value > m_gate_drop_time) &&
          (value < 600.0f)) {
          m_gate_trip_time = value;
          return true;
      }
      return false;
}

bool  gateway::has_binary_transport_flags() const noexcept
{
      return has_binary_transport_flags(emc::esf_encoding_any);
}

bool  gateway::has_binary_transport_flags(unsigned int value) const noexcept
{
      bool l_result = false;
      if((value & emc::esf_encoding_base16) != emc::esf_none) {
          l_result |= m_bt16;
      } else
      if((value & emc::esf_encoding_base64) != emc::esf_none) {
          l_result |= m_bt64;
      }
      return l_result;
}

bool  gateway::get_resume_state(bool value) const noexcept
{
      return m_resume_bit == value;
}

bool  gateway::get_healthy_state(bool value) const noexcept
{
      return m_healthy_bit == value;
}

void  gateway::flush() noexcept
{
      emc_raw_send(m_send_data, m_send_iter);
      m_send_iter = 0;
}

/*namespace emc*/ }
