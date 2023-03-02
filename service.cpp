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
#include "service.h"

namespace emc {

      service::service() noexcept:
      m_name(nullptr)
{
}

      service::service(const char* name) noexcept:
      m_name(name)
{
}

      service::~service()
{
}

bool  service::emc_dispatch_attach(session*) noexcept
{
      return true;
}

void  service::emc_dispatch_request(session*, const char*, int) noexcept
{
}

int   service::emc_process_request(session*, int, const sys::argv&) noexcept
{
      return err_no_request;
}

void  service::emc_dispatch_response(session*, const char*, int) noexcept
{
}

int   service::emc_process_response(session*, int, const sys::argv&) noexcept
{
      return err_no_response;
}

void  service::emc_dispatch_comment(session*, const char*, int) noexcept
{
}

void  service::emc_dispatch_packet(session*, int, int, std::uint8_t*) noexcept
{
}

void  service::emc_dispatch_detach(session*) noexcept
{
}

const char* service::get_name() const noexcept
{
      return m_name;
}

bool  service::get_enabled(bool value) const noexcept
{
      return value == true;
}

/*namespace emc*/ }
