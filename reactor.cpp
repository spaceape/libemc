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
#include "reactor.h"
#include <cstdlib>
#include <cstring>
#include <memory.h>
#include <pxi.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <chrono>
#include <filesystem>
#include <errno.h>
#include <fcntl.h> 

namespace emc {

      constexpr int  s_poll_count_max = 8;
      constexpr int  s_poll_event_max = 32;
      constexpr int  s_interface_limit_default = 256;
      constexpr int  s_interface_count_max = std::numeric_limits<short int>::max();
      constexpr int  s_session_limit_default = 256;
      constexpr int  s_session_count_max = std::numeric_limits<short int>::max();

      reactor::reactor() noexcept:
      m_poll_descriptor(undef),
      m_poll_count(0),
      m_interface_descriptor_map(resource::get_default(), global::cache_small_max, false, true),
      m_session_descriptor_map(resource::get_default(), global::cache_small_max, false, true),
      m_sync_time(std::chrono::steady_clock::now()),
      m_interface_count(0),
      m_interface_limit(s_interface_limit_default),
      m_session_count(0),
      m_session_limit(s_session_limit_default),
      m_ready_bit(false),
      m_resume_bit(false)
{
      int l_poll_descriptor = epoll_create(s_poll_count_max);
      if(l_poll_descriptor >= 0) {
          m_interface_list.reserve(global::cache_small_max);
          m_poll_descriptor = l_poll_descriptor;
          m_ready_bit = true;
      }
}

      reactor::~reactor()
{
      close(m_poll_descriptor);
}

bool  reactor::ctl_poll_attach(int di) noexcept
{
      struct epoll_event l_event_cb;
      if(di >= 0) {
          if(m_poll_count < s_poll_count_max) {
              l_event_cb.events  = EPOLLIN | EPOLLHUP | EPOLLERR;
              l_event_cb.data.fd = di;
              if(epoll_ctl(m_poll_descriptor, EPOLL_CTL_ADD, di, std::addressof(l_event_cb)) == 0) {
                  m_poll_count++;
                  return true;
              }
          }
      }
      return false;
}

bool  reactor::ctl_poll_detach(int di) noexcept
{
      if(di > 0) {
          if(m_poll_count > 0) {
              if(epoll_ctl(m_poll_descriptor, EPOLL_CTL_DEL, di, nullptr) == 0) {
                  m_poll_count--;
                  return true;
              }
          }
      }
      return false;
}

host* reactor::ctl_interface_find(int id) noexcept
{
      auto i_interface = m_interface_descriptor_map.find(id);
      if(i_interface != m_interface_descriptor_map.end()) {
          return i_interface->value;
      }
      return nullptr;
}

session* reactor::ctl_session_spawn(int id, host* interface) noexcept
{
      sockaddr  l_accept_ptr;
      socklen_t l_accept_length;
      int       l_accept_descriptor = accept(id, std::addressof(l_accept_ptr), std::addressof(l_accept_length));
      if(l_accept_descriptor != undef) {
          if(m_session_count < m_session_limit) {
              session* l_session_ptr = emc_spawn_session(interface, l_accept_descriptor);
              if(l_session_ptr != nullptr) {
                  bool l_session_attach = ctl_session_attach(l_accept_descriptor, l_session_ptr);
                  if(l_session_attach == true) {
                      return l_session_ptr;
                  }
              }
              ctl_poll_detach(l_accept_descriptor);
          }
          close(l_accept_descriptor);
      }
      return nullptr;
}

bool  reactor::ctl_session_attach(int id, session* session_ptr) noexcept
{
      bool l_session_attach = ctl_poll_attach(id);
      if(l_session_attach) {
          m_session_descriptor_map.insert(id, session_ptr);
          m_session_count++;
          return true;
      }
      return false;
}

void  reactor::ctl_session_drop(int id) noexcept
{
      sockaddr  l_accept_ptr;
      socklen_t l_accept_length;
      int       l_accept_descriptor = accept(id, std::addressof(l_accept_ptr), std::addressof(l_accept_length));
      if(l_accept_descriptor != undef) {
          close(l_accept_descriptor);
      }
}

session* reactor::ctl_session_find(int id) noexcept
{
      auto i_session = m_session_descriptor_map.find(id);
      if(i_session != m_session_descriptor_map.end()) {
          return i_session->value;
      }
      return nullptr;
}

void  reactor::ctl_session_feed(session* session_ptr) noexcept
{
      session_ptr->feed();
}

void  reactor::ctl_session_suspend(int id, session* session_ptr) noexcept
{
      if(auto 
          l_descriptor_iterator = m_session_descriptor_map.find(id);
          l_descriptor_iterator != m_session_descriptor_map.end()) {
          bool l_allow_suspend = emc_suspend_session(session_ptr);
          if(l_allow_suspend) {
              ctl_poll_detach(session_ptr->get_recv_descriptor());
              m_session_descriptor_map.remove(l_descriptor_iterator);
              m_session_count--;
          }
      }
}

session*  reactor::emc_spawn_session(host*, int) noexcept
{ 
      return nullptr;
}

bool  reactor::emc_suspend_session(session*) noexcept
{
      return true;
}

void  reactor::emc_sync(float) noexcept
{
}

bool  reactor::emc_attach_session(session* session_ptr) noexcept
{
      int l_id = session_ptr->get_recv_descriptor();
      if(l_id != undef) {
          return ctl_session_attach(l_id, session_ptr);
      }
      return false;
}

int   reactor::ctl_inject_session_request(session* session_ptr, const char* request, int length) noexcept
{
      return session_ptr->emc_feed_request(request, length);
}

void  reactor::emc_detach_session(session* session_ptr) noexcept
{
}

host* reactor::emc_attach_interface(host::type type, const char* address, unsigned int port) noexcept
{
      if(m_poll_count < s_poll_count_max) {
          if(m_interface_count < m_interface_limit) {
              // detect conflicting socket files - possibly from unclean exit
              if(type == host::type::socket) {
                  if(std::filesystem::is_socket(address)) {
                      if(std::filesystem::remove(address) == false) {
                          return nullptr;
                      }
                  }
              }
              // save host into the internal interface lists
              if(auto&
                  l_interface_host = m_interface_list.emplace_back(type, address, port);
                  l_interface_host.get_ready()) {
                  int  l_interface_descriptor = l_interface_host.get_descriptor();
                  bool l_interface_attach     = ctl_poll_attach(l_interface_descriptor);
                  if(l_interface_attach) {
                      m_interface_descriptor_map.insert(l_interface_descriptor, std::addressof(l_interface_host));
                      m_interface_count++;
                      return std::addressof(l_interface_host);
                  }
                  m_interface_list.pop_back();
              }
          }
      }
      return nullptr;
}

void  reactor::emc_detach_interface(host* interface_ptr) noexcept
{
      if(auto
          l_interface_iterator = m_interface_list.find(interface_ptr);
          l_interface_iterator != m_interface_list.end()) {
          ctl_poll_detach(interface_ptr->get_descriptor());
          if(auto
              l_descriptor_iterator = m_interface_descriptor_map.find(interface_ptr->get_descriptor());
              l_descriptor_iterator != m_interface_descriptor_map.end()) {
              m_interface_descriptor_map.remove(l_descriptor_iterator);
          }
          m_interface_list.erase(l_interface_iterator);
          m_interface_count--;
      }
}

bool  reactor::resume() noexcept
{
      if(m_ready_bit) {
          m_resume_bit = true;
      }
      return m_resume_bit;
}

const char* reactor::get_machine_name() const noexcept
{
      return emc_machine_name_none;
}

const char* reactor::get_machine_type() const noexcept
{
      return emc_machine_type_generic;
}

bool  reactor::suspend() noexcept
{
      if(m_resume_bit) {
          m_interface_list.clear();
          m_interface_count = 0;
          m_session_count = 0;
          m_poll_count = 0;
          m_resume_bit = false;
      }
      return m_resume_bit == false;
}

void  reactor::sync(const sys::time_t& time, const sys::delay_t& wait) noexcept
{
      epoll_event l_event_list[s_poll_event_max];

      timespec    l_wait_time;
      useconds_t  l_wait_us = std::chrono::duration_cast<std::chrono::microseconds>(wait).count();
      l_wait_time.tv_sec = l_wait_us / pxi::usps;
      l_wait_time.tv_nsec =(l_wait_us % pxi::usps) * pxi::nspus;
      if(l_wait_us > 0) {
          auto  l_dt = std::chrono::duration<float>(time - m_sync_time);
          // poll all known descriptors for signs of activity and handle incoming events
          if(int
              l_poll_rc = epoll_pwait2(
                  m_poll_descriptor,
                  l_event_list,
                  s_poll_event_max,
                  std::addressof(l_wait_time),
                  nullptr
              );
              l_poll_rc > 0) {
              for(int i_event = 0; i_event < l_poll_rc; i_event++) {
                  if(l_event_list[i_event].events & EPOLLIN) {
                      if(auto
                          l_session = ctl_session_find(l_event_list[i_event].data.fd);
                          l_session != nullptr) {
                          ctl_session_feed(l_session);
                      } else
                      if(auto
                          l_interface = ctl_interface_find(l_event_list[i_event].data.fd);
                          l_interface != nullptr) {
                          if(l_interface->has_type(host::type::socket) ||
                              l_interface->has_type(host::type::net_ipv4_address) ||
                              l_interface->has_type(host::type::net_ipv6_address) ||
                              l_interface->has_type(host::type::net_name)) {
                              if(m_resume_bit) {
                                  ctl_session_spawn(l_event_list[i_event].data.fd, l_interface);
                              } else
                                  ctl_session_drop(l_event_list[i_event].data.fd);
                              close(l_event_list[i_event].data.fd);
                          } else
                              printdbg(
                                  "Received an 'accept' event on descriptor `%d` which is not a socket.\n",
                                  __FILE__,
                                  __LINE__,
                                  l_event_list[i_event].data.fd
                              );
                      } else
                          printdbg(
                              "Received data on unregistered descriptor `%d`\n",
                              __FILE__,
                              __LINE__,
                              l_event_list[i_event].data.fd
                          );
                  } else
                  if(l_event_list[i_event].events & EPOLLHUP) {
                      if(auto
                          l_session = ctl_session_find(l_event_list[i_event].data.fd);
                          l_session != nullptr) {
                          ctl_session_suspend(l_event_list[i_event].data.fd, l_session);
                      } else
                          printdbg(
                              "Received HUP on unregistered descriptor `%d`\n",
                              __FILE__,
                              __LINE__,
                              l_event_list[i_event].data.fd
                          );
                  }
              }
          }
          // call the `sync()` method on all attached sessions
          if(true) {
              for(auto& i_session_entry : m_session_descriptor_map) {
                  session* p_session_entry = i_session_entry.value;
                  p_session_entry->sync(l_dt.count());
              }
              emc_sync(l_dt.count());
          }
          // something happened (poll wait, poll events processed), measure how long it took and recompute the
          // wait time
          auto  l_time = std::chrono::steady_clock::now();
          auto  l_elapsed = std::chrono::duration<float>(l_time - time);
          auto  l_wait = wait - l_elapsed;
          if(l_wait.count() > 0.0f) {
              useconds_t l_sleep_us = std::chrono::duration_cast<std::chrono::microseconds>(l_wait).count();
              if(l_sleep_us > 0) {
                  l_wait_us = l_sleep_us;
              } else
                  l_wait_us = 0;
          } else
              l_wait_us = 0;
          m_sync_time = time;
      }
      if(l_wait_us > 0) {
          usleep(l_wait_us);
      }
}

/*namespace emc*/ }
