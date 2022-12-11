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
#include "session.h"
#include "monitor.h"

namespace emc {

      session::session() noexcept:
      gateway()
{
      // do not reserve, monitors are a somewhat uncommon feature
      // m_monitor_list.reserve(global::cache_small_max);
}

      session::session(int rd, int sd) noexcept:
      gateway(rd, sd)
{
      emc_listen();
}

      session::~session()
{
      emc_disconnect();
}

auto  session::mon_find(monitor* monitor_ptr) noexcept -> std::vector<monitor*>::iterator
{
      if(monitor_ptr) {
          for(auto i_monitor = m_monitor_list.begin(); i_monitor != m_monitor_list.end(); i_monitor++) {
              auto& l_monitor_ptr = *i_monitor;
              if(l_monitor_ptr == monitor_ptr) {
                  return i_monitor;
              }
          }
      }
      return m_monitor_list.end();
}

void  session::emc_dispatch_request(const char* message, int length) noexcept
{
      for(monitor* i_monitor : m_monitor_list) {
          i_monitor->emc_dispatch_request(this, message, length);
      }
}

int   session::emc_process_request(int argc, command& args) noexcept
{
      int l_grc = err_no_request;
      for(monitor* i_monitor : m_monitor_list) {
          int l_mrc = i_monitor->emc_process_request(this, argc, args);
          if(l_mrc != err_no_request) {
              l_grc = l_mrc;
              // only the `err_next` rc allows the request to be dispatched to further monitors
              if(l_mrc & bit_next) {
                  break;
              }
          }
      }
      return l_grc;
}

void  session::emc_dispatch_response(const char* message, int length) noexcept
{
      for(monitor* i_monitor : m_monitor_list) {
          i_monitor->emc_dispatch_response(this, message, length);
      }
}

int   session::emc_process_response(int argc, command& args) noexcept
{
      int l_grc = err_no_request;
      for(monitor* i_monitor : m_monitor_list) {
          int l_mrc = i_monitor->emc_process_request(this, argc, args);
          if(l_mrc != err_no_request) {
              l_grc = l_mrc;
              // only the `err_next` rc allows the request to be dispatched to further monitors
              if(l_mrc & bit_next) {
                  break;
              }
          }
      }
      return l_grc;
}

void  session::emc_dispatch_comment(const char* message, int length) noexcept
{
      for(monitor* i_monitor : m_monitor_list) {
          i_monitor->emc_dispatch_comment(this, message, length);
      }
}

void  session::emc_dispatch_packet(int channel, int size, std::uint8_t* data) noexcept
{
      for(monitor* i_monitor : m_monitor_list) {
          i_monitor->emc_dispatch_packet(this, channel, size, data);
      }
}

void  session::emc_dispatch_disconnect() noexcept
{
      for(monitor* i_monitor : m_monitor_list) {
          i_monitor->emc_dispatch_disconnect(this);
      }
}

bool  session::attach(monitor* monitor_ptr) noexcept
{
      auto i_monitor = mon_find(monitor_ptr);
      if(i_monitor != m_monitor_list.end()) {
          return true;
      } else
      if(monitor_ptr != nullptr) {
          if(bool 
              l_connect_success = monitor_ptr->emc_dispatch_connect(this);
              l_connect_success == true) {
              m_monitor_list.push_back(monitor_ptr);
              return true;
          }
      }
      return false;
}

bool  session::detach(monitor* monitor_ptr) noexcept
{
      auto i_monitor = mon_find(monitor_ptr);
      if(i_monitor != m_monitor_list.end()) {
          monitor_ptr->emc_dispatch_disconnect(this);
          m_monitor_list.erase(i_monitor);
          return true;
      }
      return false;
}

/*namespace emc*/ }
