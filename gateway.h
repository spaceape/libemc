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
#include "error.h"
#include "timer.h"

namespace emc {

/* gateway
   base class for the EMC pipeline
*/
class gateway: public rawstage
{
  std::uint8_t* m_recv_data;
  int           m_recv_iter;
  int           m_recv_size;
  int           m_recv_mtu;
  int           m_recv_state;

  std::uint8_t* m_send_data;
  int           m_send_iter;
  int           m_send_size;
  int           m_send_mtu;

  sys::argv     m_args;                     // request/response command line
  emcstage*     p_stage_head;
  emcstage*     p_stage_tail;

  private:
  char          m_gate_name[emc_name_size];
  char          m_gate_info[emc_info_size];
  int           m_recv_packet_chid;
  int           m_recv_packet_left;
  int           m_recv_packet_size;
  int           m_send_packet_chid;
  int           m_send_packet_size;
  float         m_gate_ping_time;           // time of silence before triggering a ping
  float         m_gate_info_time;           // time to wait for an info response, before sending an info request
  float         m_gate_wait_time;     
  float         m_gate_drop_time;           // time of silence before any incomplete message is discarded
  float         m_gate_trip_time;           // time of silence before the machine is declared disconnected
  bool          m_bt16;
  bool          m_bt64;
  bool          m_flush_auto;               // send pending data as soon as EOL is written to the data buffer
  bool          m_flush_sync;               // send pending data upon sync() [[not implemented]]

  private:
  timer         m_ping_ctr;
  timer         m_info_ctr;                 // info timer
  timer         m_drop_ctr;                 // drop timer
  timer         m_trip_ctr;                 // trip timer
  bool          m_ping_await;               // ping has been sent, don't send another until a pong has been received

  int           m_reserve_min;              // how many bytes to reserve into the local buffers at a minimum
  int           m_reserve_max;              // how many bytes to reserve into the local buffers at most
  bool          m_stealth_bit;              // do not send status responses (acting as a proxy, maybe)

  public:
  static constexpr unsigned int o_none = 0u;
  static constexpr unsigned int o_enable_user = 1u;
  static constexpr unsigned int o_enable_host = 2u;
  static constexpr unsigned int o_remote = 8u;
  static constexpr unsigned int o_bt16 = 16u;         // translate binary to base16 (useful if transport is UART)
  static constexpr unsigned int o_bt64 = 64u;         // translate binary to base64 (useful if transport is UART)
  static constexpr unsigned int o_flush_auto = 128u;
  static constexpr unsigned int o_stealth = 256u;
  static constexpr unsigned int o_default = o_enable_host | o_remote | o_flush_auto;

  public:
  int           m_msg_recv;
  int           m_msg_drop;
  int           m_msg_tmit;
  int           m_chr_recv;
  int           m_chr_tmit;
  int           m_mem_size;
  int           m_mem_used;
  float         m_run_time;                 // how long has the gateway been up

  private:
  bool          m_host_role;                // server mode, accept and respond to requests
  bool          m_user_role;                // client mode, send requests and wait for responses
  bool          m_ping_enable;
  int           m_stage_count;
  bool          m_resume_bit;
  bool          m_join_bit;                 // raw connection is up
  bool          m_healthy_bit;              // emc connection is up and healthy

  protected:
          void  emc_emit(char) noexcept;
          void  emc_emit(int, const char*) noexcept;
          bool  emc_prepare_packet(int, int) noexcept;
          bool  emc_reserve_packet(std::uint8_t*&, int) noexcept;
          bool  emc_fill_packet(std::uint8_t*, int) noexcept;
          bool  emc_zero_packet() noexcept;
          bool  emc_emit_packet() noexcept;
          bool  emc_drop_packet() noexcept;

  private:
          auto  emc_reserve(int) noexcept -> std::uint8_t*;
          bool  emc_resume_at(emcstage*) noexcept;
          void  emc_suspend_at(emcstage*) noexcept;
          void  emc_dispatch_join() noexcept;
          void  emc_dispatch_drop() noexcept;

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

          int     emc_send_ready_response() noexcept;
          int     emc_send_info_response() noexcept;
          void    emc_send_info_request() noexcept;
          void    emc_send_service_response() noexcept;
          void    emc_send_support_response() noexcept;
          void    emc_send_ping_request() noexcept;
          int     emc_send_pong_response(const char*) noexcept;
          int     emc_send_bye_response() noexcept;
          int     emc_send_sync_response() noexcept;
          int     emc_send_help_response() noexcept;
          void    emc_send_raw(const char*, int) noexcept;
          int     emc_send_error_response(int, const char* = nullptr, ...) noexcept;

 virtual  void    emc_gate_dispatch_connect(const char*, const char*, int) noexcept;
          void    emc_gate_dispatch_message(const char*, int) noexcept;
          int     emc_gate_forward_request(int, const sys::argv&) noexcept;
          int     emc_gate_forward_response(int, const sys::argv&) noexcept;
          void    emc_gate_dispatch_comment(const char*, int) noexcept;
          int     emc_gate_forward_packet(int, int, std::uint8_t*) noexcept;
          int     emc_std_return_message(const char*, int) noexcept;
          int     emc_std_return_packet(int, int, std::uint8_t*) noexcept;
          int     emc_std_event(int, void*) noexcept;
 virtual  void    emc_gate_dispatch_disconnect() noexcept;
          void    emc_gate_drop() noexcept;
          void    emc_gate_trip() noexcept;

          auto    emc_get_gate_name() const noexcept -> const char*;
          auto    emc_get_gate_info() const noexcept -> const char*;
          int     emc_get_send_mtu() const noexcept;
          bool    emc_set_send_mtu(int) noexcept;

          int     emc_raw_feed_request(char*, int) noexcept;
          void    emc_raw_feed_help_request(char*, int) noexcept;
          void    emc_raw_feed_sync_request(char*, int) noexcept;
          void    emc_raw_feed_response(char*, int) noexcept;
          void    emc_raw_feed_comment(char*, int) noexcept;
          void    emc_raw_feed_packet(int, int, std::uint8_t*) noexcept;

  virtual void    emc_raw_attach(reactor*) noexcept override;
  virtual bool    emc_raw_resume(reactor*) noexcept override;
  virtual void    emc_raw_join() noexcept override;
  virtual int     emc_raw_feed(std::uint8_t*, int) noexcept override;
  virtual int     emc_raw_send(std::uint8_t*, int) noexcept override;
  virtual void    emc_raw_drop() noexcept override;
  virtual void    emc_raw_suspend(reactor*) noexcept override;
  virtual int     emc_raw_event(int, void*) noexcept override;
  virtual void    emc_raw_detach(reactor*) noexcept override;
  virtual void    emc_raw_sync(float) noexcept override;

  protected:
  virtual int     emc_process_request(int, const sys::argv&) noexcept;
  virtual int     emc_process_response(int, const sys::argv&) noexcept;

  friend class emcstage;
  public:
          gateway(unsigned int = o_default) noexcept;
          gateway(const gateway&) noexcept = delete;
          gateway(gateway&&) noexcept = delete;
  virtual ~gateway();

          bool    attach(emcstage*) noexcept;
          bool    detach(emcstage*) noexcept;

          bool    send_line(const char*, std::size_t = 0u) noexcept;
          bool    send_packet_raw(int, std::uint8_t*, std::size_t) noexcept;
          bool    send_packet_base16(int, std::uint8_t*, std::size_t) noexcept;
          bool    send_packet_base64(int, std::uint8_t*, std::size_t) noexcept;

          bool    set_drop_time(float) noexcept;
          bool    set_trip_time(float) noexcept;
          bool    has_binary_transport_flags() const noexcept;
          bool    has_binary_transport_flags(unsigned int) const noexcept;
          bool    get_resume_state(bool = true) const noexcept;
          bool    get_healthy_state(bool = true) const noexcept;
          
          void    flush() noexcept;
          
          gateway& operator=(const gateway&) noexcept = delete;
          gateway& operator=(gateway&&) noexcept = delete;
};

/*namespace emc*/ }
#endif
