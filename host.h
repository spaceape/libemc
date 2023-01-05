#ifndef emc_host_h
#define emc_host_h
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

/* host
   class responsible for setting up and managing an instance of a receiving endpoint connected to the EMC interface
*/
class host
{
  public:
  /* type
     accompanies an address string and describes its purpose
  */
  enum class type
  {
    none,
    file,               // host reffers to a filesystem entry
    stdio,              // host reffers to stdin
    tty,                // host reffers serial port
    socket,             // host reffers to a unix socket
    net_ipv4_address,   // host reffers to a network IPv4 address
    net_ipv6_address,   // host reffers to a network IPv6 address
    net_name            // host reffers to a network host name
  };

  /* host_mode
     records the underlying protocol a host may be using
  */
  enum class mode
  {
    null,
    emc,
    ssh,
    telnet,
    http
  };

  static constexpr unsigned int port_undef = 0;

  private:
  type          m_host_type;
  std::string   m_host_address;
  unsigned int  m_host_port;
  int           m_rd;
  int           m_sd;
  bool          m_ready_bit;

  public:
          host() noexcept;
          host(type, const char*, unsigned int = port_undef) noexcept;
          host(const host&) noexcept = delete;
          host(host&&) noexcept;
          ~host();
          bool    has_type(type) const noexcept;
          bool    has_descriptor(int) const noexcept;
          int     get_descriptor() const noexcept;
          bool    get_ready() const noexcept;
          void    dispose(bool = true) noexcept;
          void    release() noexcept;
                  operator int() const noexcept;
                  operator bool() const noexcept;
          host&   operator=(const host&) noexcept = delete;
          host&   operator=(host&&) noexcept;
};

/*namespace emc*/ }
#endif
