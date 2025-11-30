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

enum class event
{
  accept = 1,
  acquire_bus = 7,        // a stage (typically a gateway stage) obtained a descriptor (useful for polling)
  release_bus = 15,       // a stage (typically a gateway stage) needs to close a descriptor
  feed = 16,              // signal the reactor that data is available on an input/output descriptor
  pending = 20,
  listening = 23,
  running = 24,
  join = 30,
  recv = 31,              // called last on the reactor after a received message has passed through every stage in the pipeline
  send = 32,              // called last on the reactor after a message to be sent has passed through every stage in the pipeline
  progress = 36,
  terminating = 126,
  drop = 127,
  hup = 128,
  abort = 192,
  terminated = 224,
  soft_fault = 254,       // generic fault in one of the stages
  hard_fault = 255,       // generic fault in one of the stages; not recoverable, should end up with a suspend
  user_base = 256,
  user_last = 65535
};

union event_info_t
{
  int descriptor;

  struct {
    int           descriptor;
  } feed, hup, release_bus;

  struct {
    int           descriptor;
    unsigned int  events;
  } acquire_bus;

  struct {
    int           bus;
    std::uint8_t* data;
    std::size_t   size;
  } recv, send;

  char bytes[0];

  public:
  static  event_info_t for_bus_acquire(int, unsigned int) noexcept;
  static  event_info_t for_bus_release(int) noexcept;
  static  event_info_t for_feed(int) noexcept;
  static  event_info_t for_hup(int) noexcept;
  static  event_info_t for_recv(int, std::uint8_t*, std::size_t) noexcept;
  static  event_info_t for_send(int, std::uint8_t*, std::size_t) noexcept;

  public:
          event_info_t() noexcept;
          event_info_t(const event_info_t&) noexcept;
          event_info_t(event_info_t&&) noexcept;
          ~event_info_t();
          event_info_t& operator=(const event_info_t&) noexcept;
          event_info_t& operator=(event_info_t&&) noexcept;
};

/*namespace emc*/ }
#endif
