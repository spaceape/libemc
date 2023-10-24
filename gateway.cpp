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
#include "event.h"
#include "error.h"
#include <config.h>
#include <cstring>
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
      m_gate_ping_time(message_ping_time),
      m_gate_info_time(message_wait_time + message_wait_time),
      m_gate_wait_time(message_wait_time),
      m_gate_drop_time(message_drop_time),
      m_gate_trip_time(message_trip_time),
      m_error_comment(false),
      m_dispatch_packet(false),
      m_flush_auto(options & o_flush_auto),
      m_flush_sync(false),
      m_ping_ctr(false),
      m_info_ctr(false),
      m_drop_ctr(false),
      m_trip_ctr(false),
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
      m_host_role(false),
      m_user_role(false),
      m_ping_enable(options & o_remote),
      m_stage_count(0),
      m_resume_bit(false),
      m_connect_bit(false),
      m_healthy_bit(false)
{
      // reserve memory in the queues
      if constexpr (queue_size_min > 0) {
          m_recv_data = reinterpret_cast<char*>(::malloc(queue_size_min));
          m_recv_size = queue_size_min;
          m_send_data = reinterpret_cast<char*>(::malloc(queue_size_min));
          m_send_size = queue_size_min;
      }
}

      gateway::~gateway()
{
      emcstage* i_stage = p_stage_tail;
      while(i_stage != nullptr) {
          if(m_resume_bit == true) {
              if(m_connect_bit == true) {
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

void  gateway::emc_emit(int size, const char* text) noexcept
{
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

char* gateway::emc_reserve(int size) noexcept
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
              m_send_data = reinterpret_cast<char*>(l_reserve_ptr);
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

int   gateway::emc_feed_request(char* message, int length) noexcept
{
      // load the received request string onto a command pre-parser
      int   l_argc = m_args.load(message + 1);
      auto& l_argv = m_args;
      int   l_rc   = err_parse;
      if(l_argc > 0) {
          // give `emc_process_request()` priority in handling the request
          l_rc = emc_process_request(l_argc, l_argv);
          // ...or otherwise handle the protocol-mandated requests here
          if(l_rc == err_no_request) {
              if(l_argv.has_count(1)) {
                  if(l_argv[0].has_size(1)) {
                      switch(l_argv[0][0]) {
                          case emc_request_info:
                              l_rc = emc_send_info_response();
                              break;
                          case emc_request_ping:
                              l_rc = emc_send_pong_response();
                              break;
                          case emc_request_bye:
                              l_rc = emc_send_bye_response();
                              break;
                      }
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
          // on a parse error also handle as a comment, maybe?
          if(l_rc == err_parse) {
              if(m_error_comment) {
                  emc_feed_comment(message, length);
              }
          }
          l_rc = emc_send_error_response(l_rc);
      }
      return l_rc;
}

void  gateway::emc_feed_response(char* message, int length) noexcept
{
      // load the received response string onto up the command pre-parser
      int   l_argc = m_args.load(message + 1);
      auto& l_argv = m_args;
      int   l_rc   = err_parse;
      if(l_argc > 0) {
          // give `emc_process_response()` priority in handling the response
          l_rc = emc_process_response(l_argc, l_argv);
          // ...or otherwise handle standard responses here
          if(l_rc == err_no_response) {
              if(m_args[0].has_size(1)) {
                  int  l_tag = m_args[0][0];
                  switch(l_tag) {
                      case emc_response_info: {
                          if(m_args[1].has_text(emc_protocol_name)) {
                              if(m_args[2].has_text(emc_protocol_version, 0, 3)) {
                                  const char* l_name;
                                  const char* l_info;
                                  int         l_mtu;
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
                                  if(m_args[5].has_text()) {
                                      l_mtu = m_args[5].get_dec_int();
                                      std::strncpy(m_gate_name, l_name, emc_name_size);
                                      std::strncpy(m_gate_info, l_info, emc_info_size);
                                      if((l_mtu >= s_mtu_min) &&
                                          (l_mtu <= s_mtu_max)) {
                                          emc_set_send_mtu(l_mtu);
                                      }
                                      m_info_ctr.suspend();
                                      m_trip_ctr.suspend();
                                      m_trip_ctr.reset();
                                      if(m_healthy_bit == false) {
                                          emc_gate_connect(m_gate_name, m_gate_info, l_mtu);
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
                          break;
                      case emc_response_pong:
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
                      case 'a':
                      case 'b':
                      case 'c':
                      case 'd':
                      case 'e':
                      case 'f':
                      case 'A':
                      case 'B':
                      case 'C':
                      case 'D':
                      case 'E':
                      case 'F':
                          break;
                      case emc_response_bye:
                          break;
                      default:
                          break;
                  }
              }
          }
      }
      // ...or otherwise still - send an error
      if(l_rc != err_okay) {
          // on a parse error also handle as a comment, maybe?
          if(l_rc == err_parse) {
              if(m_error_comment) {
                  emc_feed_comment(message, length);
              }
          }
          // 'no response' is a silent error
          if(l_rc == err_no_response) {
              printdbg(
                  "Failed to process response.\n"
                  "    %s",
                  __FILE__,
                  __LINE__,
                  message
              );
          } else
              emc_send_error_response(l_rc);
      }
}

void  gateway::emc_feed_comment(char* message, int length) noexcept
{
      emc_gate_forward_comment(message, length);
}

void  gateway::emc_feed_packet(int channel, int size, std::uint8_t* data) noexcept
{
      emc_gate_forward_packet(channel, size, data);
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
          l_machine_name = emc_machine_name_default;
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
      emc_put(
          emc_tag_request,
          emc_request_info,
          EOL
      );
}

int   gateway::emc_send_service_response() noexcept
{
      return err_okay;
}

void  gateway::emc_send_ping_request() noexcept
{
}

int   gateway::emc_send_pong_response() noexcept
{
      return emc_send_ready_response();
}

int   gateway::emc_send_bye_response() noexcept
{
      return emc_send_ready_response();
}

int   gateway::emc_send_sync_response() noexcept
{
      emc_send_info_response();
      emc_send_service_response();
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
          case err_bad_request:
              l_message_0 = msg_bad_request;
              break;
          case err_no_request:
              l_message_0 = msg_no_request;
              break;
          default:
              break;
      }
      emc_put(emc_tag_response, fmt::X(rc, 1));
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

void  gateway::emc_gate_connect(const char*, const char*, int) noexcept
{
}

void  gateway::emc_gate_forward_message(const char* message, int length) noexcept
{
      if(p_stage_head != nullptr) {
          p_stage_head->emc_std_process_message(message, length);
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

void  gateway::emc_gate_forward_comment(const char* message, int length) noexcept
{
      if(p_stage_head != nullptr) {
          p_stage_head->emc_std_process_comment(message, length);
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

void  gateway::emc_gate_disconnect() noexcept
{
}

void  gateway::emc_std_event(int id, void* data) noexcept
{
      emc_raw_event(id, data);
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
          m_healthy_bit = false;
          m_recv_state  = s_state_drop;
      }
      m_recv_iter = 0;
      m_send_iter = 0;
      m_ping_ctr.suspend();
      m_info_ctr.suspend();
      m_drop_ctr.suspend();
      m_trip_ctr.suspend();
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

void  gateway::emc_raw_attach(reactor*) noexcept
{
}

bool  gateway::emc_raw_resume(reactor* reactor) noexcept
{
      if((m_user_role == false) &&
          (m_host_role == false)) {
          m_connect_bit = false;
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
                  m_connect_bit = true;
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
      m_drop_ctr.resume();
      if(m_user_role) {
          if(m_ping_enable) {
              if(p_owner->get_ring_flags() <= emc::reactor::emi_ring_network) {
                  m_ping_ctr.resume();
              }
          }
          m_info_ctr.resume();
          m_trip_ctr.resume();
          m_connect_bit = true;
          emc_dispatch_join();
      }
}

int   gateway::emc_raw_feed(std::uint8_t* data, int size) noexcept
{
      char* p_feed;
      char* p_last;
      int   l_read_size;
      int   l_feed_iter;
      int   l_send_rc;
      int   l_feed_rc = err_okay;
      // pass through the inbound data
      if(m_recv_state != s_state_drop) {
          l_feed_iter = 0;
          while(l_feed_iter < size) {
              p_feed      = reinterpret_cast<char*>(data + l_feed_iter);
              p_last      = reinterpret_cast<char*>(data + size);
              l_read_size = 0;
              if(m_recv_state >= s_state_accept) {
                  if(m_recv_state == s_state_accept) {
                      m_recv_iter = 0;
                      m_recv_packet_left = 0;
                      m_recv_packet_size = 0;
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
                      // run through the inbound data and copy it into the receive buffer, until the first end of line;
                      while(p_feed < p_last) {
                          l_read_size++;
                          if(m_recv_iter == m_recv_size) {
                              void* l_recv_data;
                              int   l_recv_reserve;
                              if(m_recv_iter >= m_reserve_max) {
                                  l_reject     = true;
                                  l_feed_rc = err_fail;
                                  break;
                              }
                              l_recv_reserve = m_recv_size + size;
                              if(l_recv_reserve > m_reserve_max) {
                                  l_recv_reserve = m_reserve_max;
                              }
                              l_recv_data = ::realloc(m_recv_data, l_recv_reserve);
                              if(l_recv_data == nullptr) {
                                  l_reject     = true;
                                  l_feed_rc = err_fail;
                                  break;
                              }
                              m_recv_size = l_recv_reserve;
                          }
                          if(*p_feed == NUL) {
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
                          char* p_recv        = m_recv_data;
                          int   l_recv_length = m_recv_iter;
                          if((m_user_role == true) ||
                              (m_host_role == true)) {
                              if(m_host_role == true) {
                                  // capture requests, if gateway is running as server
                                  if(p_recv[0] == emc_tag_request) {
                                      emc_feed_request(p_recv, l_recv_length);
                                  } else
                                  if((p_recv[0] == emc_tag_sync) && (p_recv[1] == NUL)) {
                                      emc_send_sync_response();
                                  } else
                                  if((p_recv[0] == emc_tag_help) && (p_recv[1] == NUL)) {
                                      emc_send_help_response();
                                  } else
                                      emc_feed_comment(p_recv, l_recv_length);
                              } else
                              if(m_user_role == true) {
                                  // capture responses, if waiting for any
                                  if(p_recv[0] == emc_tag_response) {
                                      emc_feed_response(p_recv, l_recv_length);
                                  } else
                                      emc_feed_comment(p_recv, l_recv_length);
                              }
                          } else
                          if((m_user_role == false) &&
                              (m_host_role == false)) {
                              emc_gate_forward_message(p_recv, l_recv_length);
                          }
                          m_recv_iter = 0;
                          m_recv_state = s_state_accept;
                          m_msg_recv++;
                      }
                      // clear the message drop timer 
                      m_drop_ctr.reset();
                  } else
                  if(m_recv_state == s_state_capture_packet) {
                      if(m_recv_packet_left <= size) {
                          l_read_size = m_recv_packet_left;
                      } else
                          l_read_size = size;
                      // TODO - packet support
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
                          if((*p_feed == NUL) || (*p_feed == EOL)) {
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
      emc_dispatch_drop();
      // external event implies that the connection with the endpoint has been lost
      if(m_user_role) {
          m_healthy_bit = false;
          m_connect_bit = false;
          m_recv_state  = s_state_drop;
          m_user_role   = false;
      }
      // drop the messages inbound and outbound messages from their respective queues
      m_recv_iter = 0;
      m_send_iter = 0;
      // suspend timers
      m_ping_ctr.suspend();
      m_info_ctr.suspend();
      m_drop_ctr.suspend();
      m_trip_ctr.suspend();
}

void  gateway::emc_raw_suspend(emc::reactor*) noexcept
{
      // external event wants us to suspend operation
      if(m_resume_bit) {
          if(m_connect_bit) {
              emc_dispatch_drop();
              if(m_healthy_bit) {
                  m_healthy_bit = false;
              }
              m_connect_bit = false;
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
          m_resume_bit = false;
      }
}

void  gateway::emc_raw_event(int id, void* stage) noexcept
{
      rawstage::emc_raw_event(id, stage);
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
                  if(m_connect_bit) {
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
                  if(m_connect_bit) {
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

void  gateway::flush() noexcept
{
      emc_raw_send(reinterpret_cast<std::uint8_t*>(m_send_data), m_send_iter);
      m_send_iter = 0;
}

/*namespace emc*/ }
