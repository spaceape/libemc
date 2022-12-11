#ifndef emc_session_h
#define emc_session_h
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
#include "protocol.h"
#include "gateway.h"
#include <vector>

namespace emc {

/* session
*/
class session: public gateway
{
  std::vector<monitor*> m_monitor_list;

  protected:
          auto  mon_find(monitor*) noexcept -> std::vector<monitor*>::iterator;

  virtual void  emc_dispatch_request(const char*, int) noexcept override;
  virtual int   emc_process_request(int, command&) noexcept override;
  virtual void  emc_dispatch_response(const char*, int) noexcept override;
  virtual int   emc_process_response(int, command&) noexcept override;
  virtual void  emc_dispatch_comment(const char*, int) noexcept override;
  virtual void  emc_dispatch_packet(int, int, std::uint8_t*) noexcept override;
  virtual void  emc_dispatch_disconnect() noexcept override;

  friend class monitor;
  public:
          session() noexcept;
          session(int, int) noexcept;
          session(const session&) noexcept = delete;
          session(session&&) noexcept = delete;
          ~session();
          bool     attach(monitor*) noexcept;
          bool     detach(monitor*) noexcept;
          session& operator=(const session&) noexcept = delete;
          session& operator=(session&&) noexcept = delete;
};

/*namespace emc*/ }
#endif
