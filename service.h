#ifndef emc_service_h
#define emc_service_h
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
#include "feature.h"
#include "gateway.h"

namespace emc {

class service
{
  protected:
  virtual bool  emc_dispatch_attach(session*) noexcept;
  virtual void  emc_dispatch_request(session*, const char*, int) noexcept;
  virtual int   emc_process_request(session*, int, const sys::argv&) noexcept;
  virtual void  emc_dispatch_response(session*, const char*, int) noexcept;
  virtual int   emc_process_response(session*, int, const sys::argv&) noexcept;
  virtual void  emc_dispatch_comment(session*, const char*, int) noexcept;
  virtual void  emc_dispatch_packet(session*, int, int, std::uint8_t*) noexcept;
  virtual void  emc_dispatch_detach(session*) noexcept;

  friend class session;
  public:
          service() noexcept;
          service(const service&) noexcept = delete;
          service(service&&) noexcept = delete;
          ~service();
  virtual const char* get_name() const noexcept;
  virtual feature*    get_feature_ptr(int) noexcept;
  virtual int         get_feature_count() const noexcept;
  virtual bool        get_enabled(bool = true) const noexcept;
          service& operator=(const service&) noexcept = delete;
          service& operator=(service&&) noexcept = delete;
};

/*namespace emc*/ }
#endif
