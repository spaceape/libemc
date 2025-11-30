/**
    Copyright (c) 2025, wicked systems
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
#include "event.h"
#include <string>

namespace emc {

      event_info_t::event_info_t() noexcept
{
      std::memset(bytes, 0, sizeof(event_info_t));
}

      event_info_t::event_info_t(const event_info_t& copy) noexcept
{
      std::memcpy(bytes, copy.bytes, sizeof(event_info_t));
}

      event_info_t::event_info_t(event_info_t&& copy) noexcept
{
      std::memcpy(bytes, copy.bytes, sizeof(event_info_t));
      std::memset(copy.bytes, 0, sizeof(event_info_t));
}

      event_info_t::~event_info_t()
{
}

event_info_t event_info_t::for_bus_acquire(int descriptor, unsigned int events) noexcept
{
      event_info_t l_result;
      l_result.acquire_bus.descriptor = descriptor;
      l_result.acquire_bus.events = events;
      return l_result;
}

event_info_t event_info_t::for_bus_release(int descriptor) noexcept
{
      event_info_t l_result;
      l_result.release_bus.descriptor = descriptor;
      return l_result;
}

event_info_t event_info_t::for_feed(int descriptor) noexcept
{
      event_info_t l_result;
      l_result.feed.descriptor = descriptor;
      return l_result;
}

event_info_t event_info_t::for_hup(int descriptor) noexcept
{
      event_info_t l_result;
      l_result.hup.descriptor = descriptor;
      return l_result;
}

event_info_t event_info_t::for_recv(int bus, std::uint8_t* data, std::size_t size) noexcept
{
      event_info_t l_result;
      l_result.recv.bus = bus;
      l_result.recv.data = data;
      l_result.recv.size = size;
      return l_result;
}

event_info_t event_info_t::for_send(int bus, std::uint8_t* data, std::size_t size) noexcept
{
      event_info_t l_result;
      l_result.send.bus = bus;
      l_result.send.data = data;
      l_result.send.size = size;
      return l_result;
}

event_info_t& event_info_t::operator=(const event_info_t& rhs) noexcept
{
      if(std::addressof(rhs) != this) {
          std::memcpy(bytes, rhs.bytes, sizeof(event_info_t));
      }
      return *this;
}

event_info_t& event_info_t::operator=(event_info_t&& rhs) noexcept
{
      if(std::addressof(rhs) != this) {
          std::memcpy(bytes, rhs.bytes, sizeof(event_info_t));
          std::memset(rhs.bytes, 0, sizeof(event_info_t));
      }
      return *this;
}

/*namespace emc*/ }
