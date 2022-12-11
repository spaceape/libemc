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
#include "device.h"
#include <sys/un.h>
#include <sys/socket.h>
#include <cstring>
#include <filesystem>
#include <unistd.h>
#include <fcntl.h> 
#include <errno.h>
#include <error.h>

namespace emc {

      host::host(host::type type, const std::string& address) noexcept:
      m_host_type(host::type::none),
      m_host_address(),
      m_recv_descriptor(undef),
      m_connect_bit(false),
      m_ready_bit(false)
{
      if(type == host::type::stdio) {
          m_recv_descriptor = STDIN_FILENO;
          m_host_type = type;
          m_connect_bit = true;
          m_ready_bit = true;
      } else
      if(type == host::type::socket) {
          m_recv_descriptor = ::socket(AF_UNIX, SOCK_STREAM, 0);
          if(m_recv_descriptor >= 0) {
              sockaddr_un l_info;
              l_info.sun_family = AF_UNIX;
              std::strncpy(l_info.sun_path, address.c_str(), sizeof(l_info.sun_path) - 1);
              // bind to socket file and mark the reactor as ready
              if(::bind(m_recv_descriptor, reinterpret_cast<sockaddr*>(std::addressof(l_info)), sizeof(sockaddr_un)) == 0) {
                  if(::listen(m_recv_descriptor, 0) == 0) {
                      m_host_type    = type;
                      m_host_address = address;
                      m_connect_bit  = true;
                      m_ready_bit    = true;
                  } else
                      printdbg(
                          "[%p] Unable to listen on socket descriptor `%d`",
                          __FILE__,
                          __LINE__,
                          this,
                          m_recv_descriptor
                      );
              } else
                  printdbg(
                      "[%p] Unable to bind socket descriptor `%d`",
                      __FILE__,
                      __LINE__,
                      this,
                      m_recv_descriptor
                  );
          }
      } else
          printdbg(
              "[%p] Host type `%d` not currently supported.",
              __FILE__,
              __LINE__,
              this,
              type
          );
}

      host::host(host&& copy) noexcept
{
}

      host::~host()
{
      if(m_recv_descriptor >= 0) {
          if(m_recv_descriptor != STDIN_FILENO) {
              // socket host type: remove the socket file from the filesystem
              if(m_host_type == host::type::socket) {
                  std::filesystem::remove(m_host_address);
              }
              close(m_recv_descriptor);
          }
      }
}

bool  host::has_type(type value) const noexcept
{
      return m_host_type == value;
}

bool  host::has_descriptor(int id) const noexcept
{
      return m_recv_descriptor == id;
}

int   host::get_descriptor() const noexcept
{
      return m_recv_descriptor;
}

bool  host::get_ready() const noexcept
{
      return m_ready_bit;
}

      host::operator int() const noexcept
{
      return m_recv_descriptor;
}

host& host::operator=(host&& rhs) noexcept
{
      return *this;
}

/*namespace emc*/ }
