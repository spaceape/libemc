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
#include "monitor.h"

namespace emc {

      monitor::monitor() noexcept
{
}

      monitor::~monitor() noexcept
{
}

bool  monitor::emc_dispatch_connect(session*) noexcept
{
      return true;
}

void  monitor::emc_dispatch_request(session*, const char*, int) noexcept
{
}

int   monitor::emc_process_request(session*, int, command&) noexcept
{
      return err_no_request;
}

void  monitor::emc_dispatch_response(session*, const char*, int) noexcept
{
}

int   monitor::emc_process_response(session*, int, command&) noexcept
{
      return err_no_response;
}

void  monitor::emc_dispatch_comment(session*, const char*, int) noexcept
{
}

void  monitor::emc_dispatch_packet(session*, int, int, std::uint8_t*) noexcept
{
}

void  monitor::emc_dispatch_disconnect(session*) noexcept
{
}

/*namespace emc*/ }
