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
#include "service.h"
#include <config.h>

namespace emc {

      session::session(unsigned int type) noexcept:
      gateway(),
      m_service_flags(type)
{
      m_service_list.reserve(global::cache_small_max);
}

      session::session(unsigned int type, int rd, int sd) noexcept:
      gateway(rd, sd),
      m_service_flags(type)
{
      emc_listen();
}

      session::~session()
{
      emc_disconnect();
}

auto  session::svc_find(service* service_ptr) noexcept -> std::vector<service*>::iterator
{
      if(service_ptr) {
          for(auto i_service = m_service_list.begin(); i_service != m_service_list.end(); i_service++) {
              auto& l_service_ptr = *i_service;
              if(l_service_ptr == service_ptr) {
                  return i_service;
              }
          }
      }
      return m_service_list.end();
}

void  session::emc_dispatch_request(const char* message, int length) noexcept
{
      for(service* i_service : m_service_list) {
          i_service->emc_dispatch_request(this, message, length);
      }
}

int   session::emc_process_request(int argc, command& args) noexcept
{
      int l_grc = err_no_request;
      for(service* i_service : m_service_list) {
          int l_mrc = i_service->emc_process_request(this, argc, args);
          if(l_mrc != err_no_request) {
              l_grc = l_mrc;
              // only the `err_next` rc allows the request to be dispatched to further services
              if(l_mrc & bit_next) {
                  break;
              }
          }
      }
      return l_grc;
}

void  session::emc_dispatch_response(const char* message, int length) noexcept
{
      for(service* i_service : m_service_list) {
          i_service->emc_dispatch_response(this, message, length);
      }
}

int   session::emc_process_response(int argc, command& args) noexcept
{
      int l_grc = err_no_request;
      for(service* i_service : m_service_list) {
          int l_mrc = i_service->emc_process_request(this, argc, args);
          if(l_mrc != err_no_request) {
              l_grc = l_mrc;
              // only the `err_next` rc allows the request to be dispatched to further services
              if(l_mrc & bit_next) {
                  break;
              }
          }
      }
      return l_grc;
}

void  session::emc_dispatch_comment(const char* message, int length) noexcept
{
      for(service* i_service : m_service_list) {
          i_service->emc_dispatch_comment(this, message, length);
      }
}

void  session::emc_dispatch_packet(int channel, int size, std::uint8_t* data) noexcept
{
      for(service* i_service : m_service_list) {
          i_service->emc_dispatch_packet(this, channel, size, data);
      }
}

void  session::emc_dispatch_disconnect() noexcept
{
      for(service* i_service : m_service_list) {
          i_service->emc_dispatch_detach(this);
      }
}

bool  session::attach(service* service_ptr) noexcept
{
      auto i_service = svc_find(service_ptr);
      if(i_service != m_service_list.end()) {
          return true;
      } else
      if(service_ptr != nullptr) {
          int l_service_count = m_service_list.size();
          if(l_service_count < service_count_max) {
              if(bool 
                  l_connect_success = service_ptr->emc_dispatch_attach(this);
                  l_connect_success == true) {
                  // save the service into the internal service index
                  m_service_list.push_back(service_ptr);
                  // notify the client of the new service, if it is enabled
                  emc_send_service_event(ssf_enable, service_ptr);
                  return true;
              }
          }
      }
      return false;
}

bool  session::detach(service* service_ptr) noexcept
{
      auto i_service = svc_find(service_ptr);
      if(i_service != m_service_list.end()) {
          // notify the client of the detaching service, if it is enabled
          emc_send_service_event(ssf_disable, service_ptr);
          // remove the service from the internal service index
          service_ptr->emc_dispatch_detach(this);
          m_service_list.erase(i_service);
          return true;
      }
      return false;
}

service* session::get_service_ptr(int index) noexcept
{
      if(index >= 0) {
          int l_service_count = m_service_list.size();
          if(index < l_service_count) {
              return m_service_list[index];
          }
      }
      return nullptr;
}

int   session::get_service_count() const noexcept
{
      return m_service_list.size();
}

bool  session::has_service_flags(unsigned int flags, unsigned int expect) const noexcept
{
      return (m_service_flags & flags) == (m_service_flags & expect);
}

auto  session::get_service_flags() const noexcept
{
      return m_service_flags;
}

/*namespace emc*/ }
