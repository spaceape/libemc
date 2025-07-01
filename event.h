#ifndef emc_event_h
#define emc_event_h
/**
    Copyright (c) 2022, wicked systems
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

namespace emc {

/* ev_accept
*/
static constexpr int  ev_accept = 1;

/* ev_feed
   signal the reactor that data is available on an input descriptor
*/
static constexpr int  ev_feed = 2;

/* ev_pending
*/
static constexpr int  ev_pending = 5;

/* ev_running
*/
static constexpr int  ev_running = 6;

/* ev_join
   signal the reactor that the endpoint connection is up
*/
static constexpr int  ev_join = 7;

/* ev_connect
*/
static constexpr int  ev_connect = 8;

/* ev_send
*/
static constexpr int  ev_send = 15;

/* ev_recv_std
*/
static constexpr int  ev_recv_std = 31;

/* ev_recv_std
*/
static constexpr int  ev_recv_aux = 32;

/* ev_progress
*/
static constexpr int  ev_progress = 36;

/* ev_disconnect
*/
static constexpr int  ev_disconnect = 124;

/* ev_terminating
*/
static constexpr int  ev_terminating = 126;

/* ev_drop
   signal the reactor that the endpoint connection is down
*/
static constexpr int  ev_drop = 127;

/* ev_hup
   signal the reactor that the endpoint has hung up
*/
static constexpr int  ev_hup = 128;

/* ev_close_request
   signal the reactor that the endpoint requests a close
*/
static constexpr int  ev_close_request = 132;

/* ev_reset_request
   signal the reactor that the application requested a pipeline reset
*/
static constexpr int  ev_reset_request = 133;

/* ev_abort
*/
static constexpr int  ev_abort = 192;

/* ev_terminated
*/
static constexpr int  ev_terminated = 224;

/* ev_soft_fault
   generic fault in one of the stages
*/
static constexpr int  ev_soft_fault = 254;

/* ev_hard_fault
   generic fault in one of the stages; not recoverable, should end up with a suspend
*/
static constexpr int  ev_hard_fault = 255;

/* ev_user_base
*/
static constexpr int  ev_user_base = 256;

/* ev_user_last
*/
static constexpr int  ev_user_last = 65535;

/* event
*/
union event_t
{
  struct {
    int value;
  } descriptor;

  struct {
    char*         content;
    std::size_t   length;
  } message;

  struct {
    std::uint8_t* data;
    std::size_t   size;
  } packet;

  char bytes[0];

  public:
  static  event_t for_descriptor(int) noexcept;
  static  event_t for_message(char*, std::size_t) noexcept;
  static  event_t for_packet(std::uint8_t*, std::size_t) noexcept;

  public:
          event_t() noexcept;
          event_t(const event_t&) noexcept;
          event_t(event_t&&) noexcept;
          ~event_t();
          event_t& operator=(const event_t&) noexcept;
          event_t& operator=(event_t&&) noexcept;
};

/*namespace emc*/ }
#endif
