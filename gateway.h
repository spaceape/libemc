#ifndef emc_gateway_h
#define emc_gateway_h
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
#include "emc.h"
#include "pipeline.h"
#include "protocol.h"
#include "error.h"
#include "timer.h"

namespace emc {

/* gateway
   base class for an EMC pipeline
*/
class gateway: public rawstage
{
  char*     m_recv_data;
  int       m_recv_iter;
  int       m_recv_size;
  int       m_recv_packet_left;
  int       m_recv_packet_size;
  int       m_recv_mtu;
  int       m_recv_state;

  char*     m_send_data;
  int       m_send_iter;
  int       m_send_size;
  int       m_send_mtu;

  sys::argv m_args;               // request/response command line
  emcstage* p_stage_head;
  emcstage* p_stage_tail;

  private:
  char      m_gate_name[emc_name_size];
  char      m_gate_info[emc_info_size];
  float     m_gate_ping_time;           // time of silence before triggering a ping
  float     m_gate_info_time;           // time to wait for an info response, before sending an info request
  float     m_gate_wait_time;     
  float     m_gate_drop_time;           // time of silence before any incomplete message is discarded
  float     m_gate_trip_time;           // time of silence before the machine is declared disconnected
  bool      m_error_comment;            // if a request fails to parse correctly - handle it as a comment
  bool      m_dispatch_packet;
  bool      m_flush_auto;               // send pending data as soon as EOL is written to the data buffer
  bool      m_flush_sync;               // send pending data upon sync() [[not implemented]]

  private:
  timer     m_ping_ctr;
  timer     m_info_ctr;                 // info timer    
  timer     m_drop_ctr;                 // drop timer
  timer     m_trip_ctr;                 // trip timer

  int       m_reserve_min;              // how many bytes to reserve into the local buffers at a minimum
  int       m_reserve_max;              // how many bytes to reserve into the local buffers at most
  bool      m_stealth_bit;              // do not send status responses (acting as a proxy, maybe)

  public:
  static constexpr unsigned int o_none = 0u;
  static constexpr unsigned int o_enable_user = 1u;
  static constexpr unsigned int o_enable_host = 2u;
  static constexpr unsigned int o_flush_auto = 16u;
  static constexpr unsigned int o_stealth = 256u;
  static constexpr unsigned int o_default = o_enable_host | o_enable_user | o_flush_auto;

  public:
  int       m_msg_recv;
  int       m_msg_drop;
  int       m_msg_tmit;
  int       m_chr_recv;
  int       m_chr_tmit;
  int       m_mem_size;
  int       m_mem_used;

  private:
  bool      m_host_role;                // server mode, accept and respond to requests
  bool      m_user_role;                // client mode, send requests and wait for responses
  int       m_stage_count;
  bool      m_ready_bit;                // raw connection is up
  bool      m_healthy_bit;              // emc connection is up and healthy

  private:
          void    emc_emit(char) noexcept;
          void    emc_emit(int, const char*) noexcept;
          char*   emc_reserve(int) noexcept;

          int     emc_feed_request(char*, int) noexcept;
          void    emc_feed_response(char*, int) noexcept;
          void    emc_feed_comment(char*, int) noexcept;
          void    emc_feed_packet(int, int, std::uint8_t*) noexcept;

  protected:
  inline  void  emc_put() noexcept {
  }

  template<typename... Args>
  inline  void  emc_put(char c, Args&&... next) noexcept {
          emc_emit(c);
          emc_put(std::forward<Args>(next)...);
  }

  template<typename... Args>
  inline  void  emc_put(const char* text, Args&&... next) noexcept {
          emc_emit(0, text);
          emc_put(std::forward<Args>(next)...);
  }

  template<typename... Args>
  inline  void  emc_put(const fmt::d& value, Args&&... next) noexcept {
          emc_emit(0, value);
          emc_put(std::forward<Args>(next)...);
  }

  template<typename... Args>
  inline  void  emc_put(const fmt::x& value, Args&&... next) noexcept {
          emc_emit(0, value);
          emc_put(std::forward<Args>(next)...);
  }

  template<typename... Args>
  inline  void  emc_put(const fmt::X& value, Args&&... next) noexcept {
          emc_emit(0, value);
          emc_put(std::forward<Args>(next)...);
  }

  template<typename... Args>
  inline  void  emc_put(const fmt::f& value, Args&&... next) noexcept {
          emc_emit(0, value);
          emc_put(std::forward<Args>(next)...);
  }

          int     emc_send_info_response() noexcept;
          void    emc_send_info_request() noexcept;
          int     emc_send_service_response() noexcept;
          void    emc_send_ping_request() noexcept;
          int     emc_send_pong_response() noexcept;
          int     emc_send_bye_response() noexcept;
          int     emc_send_sync_response() noexcept;
          int     emc_send_help_response() noexcept;
          void    emc_send_raw(const char*, int) noexcept;
          int     emc_send_error(int, const char* = nullptr, ...) noexcept;

          void    emc_std_join() noexcept;
          void    emc_std_connect(const char*, const char*, int) noexcept;
          int     emc_std_forward_request(int, const sys::argv&) noexcept;
          int     emc_std_forward_response(int, const sys::argv&) noexcept;
          void    emc_std_forward_comment(const char*, int) noexcept;
          int     emc_std_forward_packet(int, int, std::uint8_t*) noexcept;
          int     emc_std_return_message(const char*, int) noexcept;
          int     emc_std_return_packet(int, int, std::uint8_t*) noexcept;
          void    emc_std_disconnect() noexcept;
          void    emc_std_trip() noexcept;
          void    emc_std_drop() noexcept;

          auto    emc_get_gate_name() const noexcept -> const char*;
          auto    emc_get_gate_info() const noexcept -> const char*;
          int     emc_get_send_mtu() const noexcept;
          bool    emc_set_send_mtu(int) noexcept;

  virtual void    emc_raw_attach(reactor*) noexcept override;
  virtual void    emc_raw_join() noexcept override;
  virtual int     emc_raw_feed(std::uint8_t*, int) noexcept override;
  virtual int     emc_raw_send(std::uint8_t*, int) noexcept override;
  virtual void    emc_raw_drop() noexcept override;
  virtual void    emc_raw_detach(reactor*) noexcept override;

  protected:
  virtual int     emc_process_request(int, const sys::argv&) noexcept;
  virtual int     emc_process_response(int, const sys::argv&) noexcept;
  virtual void    emc_stage_attach(int, emcstage*) noexcept;
  virtual void    emc_stage_detach(int, emcstage*) noexcept;

  friend class emcstage;
  public:
          gateway(unsigned int = o_default) noexcept;
          gateway(const gateway&) noexcept = delete;
          gateway(gateway&&) noexcept = delete;
  virtual ~gateway();
          bool    attach(emcstage*) noexcept;
          bool    detach(emcstage*) noexcept;
          void    flush() noexcept;
          void    reset() noexcept;
  virtual void    sync(float) noexcept override final;
          gateway& operator=(const gateway&) noexcept = delete;
          gateway& operator=(gateway&&) noexcept = delete;
};

/*namespace emc*/ }
#endif
