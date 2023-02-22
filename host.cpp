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
#include "host.h"
#include <cstring>
#include <filesystem>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>

namespace emc {

      host::host() noexcept:
      m_host_type(host::type::none),
      m_host_address(),
      m_host_port(0),
      m_rd(undef),
      m_sd(undef),
      m_ready_bit(false)
{
}

      host::host(host::type type, const char* address, unsigned int port) noexcept:
      host()
{
      if(type == host::type::stdio) {
          m_rd = STDIN_FILENO;
          m_sd = STDOUT_FILENO;
          m_host_type = host::type::stdio;
          printdbg("Host [%p]: Enabling STDIO descriptors %d, %d.", m_rd, m_sd);
          m_ready_bit = true;
      } else
      if(type == host::type::tty) {
          m_rd = ::open(address, O_RDWR | O_NOCTTY | O_SYNC);
          if(m_rd >= 0) {
              m_host_type    = host::type::tty;
              m_host_address = address;
              m_sd           = m_rd;
              printdbg("Host [%p]: Listening on tty \"%s\".", address);
              m_ready_bit    = true;
          }
      } else
      if(type == host::type::socket) {
          m_rd = ::socket(AF_UNIX, SOCK_STREAM, 0);
          if(m_rd >= 0) {
              struct sockaddr_un l_sa;
              l_sa.sun_family = AF_UNIX;
              std::strncpy(l_sa.sun_path, address, sizeof(l_sa.sun_path) - 1);
              if(::bind(m_rd, reinterpret_cast<sockaddr*>(std::addressof(l_sa)), sizeof(sockaddr_un)) == 0) {
                  if(::listen(m_rd, 0) == 0) {
                      m_host_type    = host::type::socket;
                      m_host_address = address;
                      m_sd           = m_rd;
                      printdbg("Host [%p]: Listening on UNIX socket \"%s\".", this, address);
                      m_ready_bit    = true;
                  } else
                      printdbg(
                          "[%p] Unable to listen on socket descriptor `%d`",
                          __FILE__,
                          __LINE__,
                          this,
                          m_rd
                      );
              } else
                  printdbg(
                      "[%p] Unable to bind socket descriptor `%d`",
                      __FILE__,
                      __LINE__,
                      this,
                      m_rd
                  );
          } else
              printdbg(
                  "[%p] Unable to create socket descriptor",
                  __FILE__,
                  __LINE__,
                  this
              );
      } else
      if((type == host::type::net_ipv4_address) ||
          (type == host::type::net_ipv6_address)) {
          if((port > 0) &&
              (port < 65536)) {
              host::type          l_net_type = type;
              struct sockaddr_in  l_ipv4_sa;
              struct sockaddr_in6 l_ipv6_sa;
              struct sockaddr*    l_sa_ptr = nullptr;
              std::size_t         l_sa_size = 0;
              if(l_net_type == host::type::net_ipv4_address) {
                  if(inet_pton(AF_INET, address, std::addressof(l_ipv4_sa.sin_addr)) == 1) {
                      m_rd = ::socket(AF_INET, SOCK_STREAM, 0);
                      if(m_rd >= 0) {
                          l_ipv4_sa.sin_family = AF_INET;
                          l_ipv4_sa.sin_port   = htons(port);
                          l_sa_ptr  = reinterpret_cast<struct sockaddr*>(std::addressof(l_ipv4_sa));
                          l_sa_size = sizeof(l_ipv4_sa);
                      } else
                          printdbg(
                              "[%p] Unable to create socket descriptor",
                              __FILE__,
                              __LINE__,
                              this
                          );
                  } else
                      printdbg(
                          "[%p] Socket address `%s` is invalid",
                          __FILE__,
                          __LINE__,
                          this,
                          address
                      );
              } else
              if(l_net_type == host::type::net_ipv6_address) {
                  if(inet_pton(AF_INET6, address, std::addressof(l_ipv6_sa.sin6_addr)) == 1) {
                      m_rd = ::socket(AF_INET6, SOCK_STREAM, 0);
                      if(m_rd >= 0) {
                          l_ipv6_sa.sin6_family = AF_INET;
                          l_ipv6_sa.sin6_port   = htons(port);
                          l_sa_ptr  = reinterpret_cast<struct sockaddr*>(std::addressof(l_ipv6_sa));
                          l_sa_size = sizeof(l_ipv6_sa);
                      } else
                          printdbg(
                              "[%p] Unable to create socket descriptor",
                              __FILE__,
                              __LINE__,
                              this
                          );
                  } else
                      printdbg(
                          "[%p] Socket address `%s` is invalid",
                          __FILE__,
                          __LINE__,
                          this,
                          address
                      );
              }
              if(l_sa_ptr != nullptr) {
                  if(::bind(m_rd, l_sa_ptr, l_sa_size) == 0) {
                      if(::listen(m_rd, 0) == 0) {
                          m_host_type    = type;
                          m_host_address = address;
                          m_host_port    = port;
                          m_sd           = m_rd;
                          printdbg("Host [%p]: Listening on INET socket \"%s:%d\".", this, address, port);
                          m_ready_bit    = true;
                      } else
                          printdbg(
                              "[%p] Unable to listen on socket descriptor `%d`",
                              __FILE__,
                              __LINE__,
                              this,
                              m_rd
                          );
                  } else
                      printdbg(
                          "[%p] Unable to bind socket descriptor `%d`",
                          __FILE__,
                          __LINE__,
                          this,
                          m_rd
                      );
              }
          } else
              printdbg(
                  "[%p] Socket port number `%d` is invalid",
                  __FILE__,
                  __LINE__,
                  this,
                  port
              );
      } else
          printdbg(
              "[%p] Host type `%d` not currently supported",
              __FILE__,
              __LINE__,
              this,
              type
          );
}

      host::host(host&& copy) noexcept:
      m_host_type(copy.m_host_type),
      m_host_address(copy.m_host_address),
      m_host_port(copy.m_host_port),
      m_rd(copy.m_rd),
      m_sd(copy.m_sd),
      m_ready_bit(copy.m_ready_bit)
{
      copy.release();
}

      host::~host()
{
      dispose(false);
}

bool  host::has_type(type value) const noexcept
{
      return m_host_type == value;
}

bool  host::has_descriptor(int id) const noexcept
{
      return m_rd == id;
}

int   host::get_descriptor() const noexcept
{
      return m_rd;
}

bool  host::get_ready() const noexcept
{
      return m_ready_bit;
}

void  host::dispose(bool clear) noexcept
{
      if(m_rd >= 0) {
          if(m_sd >= 0) {
              if(m_sd != STDOUT_FILENO) {
                  if(m_sd != m_rd) {
                      close(m_sd);
                  }
              }
          }
          if(m_rd != STDIN_FILENO) {
              // socket host type: remove the socket file from the filesystem
              if(m_host_type == host::type::socket) {
                  std::filesystem::remove(m_host_address);
              }
              close(m_rd);
          }
      }
      if(clear) {
          release();
      }
}

void  host::release() noexcept
{
      m_host_type = host::type::none;
      m_rd = undef;
      m_sd = undef;
}

      host::operator bool() const noexcept
{
      return m_host_type != host::type::none;
}

      host::operator int() const noexcept
{
      return m_rd;
}

host& host::operator=(host&& rhs) noexcept
{
      if(std::addressof(rhs) != this) {
          dispose(false);
          m_host_type = rhs.m_host_type;
          m_host_address = rhs.m_host_address;
          m_host_port = rhs.m_host_port;
          m_rd = rhs.m_rd;
          m_sd = rhs.m_sd;
          m_ready_bit = rhs.m_ready_bit;
          rhs.release();
      }
      return *this;
}

/*namespace emc*/ }
