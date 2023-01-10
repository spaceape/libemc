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
  unsigned int          m_service_flags;
  std::vector<service*> m_service_list;

  private:
          auto  svc_find(service*) noexcept -> std::vector<service*>::iterator;

  protected:
  virtual void  emc_dispatch_request(const char*, int) noexcept override;
  virtual int   emc_process_request(int, const sys::argv&) noexcept override;
  virtual void  emc_dispatch_response(const char*, int) noexcept override;
  virtual int   emc_process_response(int, const sys::argv&) noexcept override;
  virtual void  emc_dispatch_comment(const char*, int) noexcept override;
  virtual void  emc_dispatch_packet(int, int, std::uint8_t*) noexcept override;
  virtual void  emc_dispatch_disconnect() noexcept override;

  public:
  static constexpr unsigned int bit_none = 0u;
  static constexpr unsigned int bit_local = 1u;                   // session binds a local resource
  static constexpr unsigned int bit_network = 2u;                 // session binds a network resource
  static constexpr unsigned int bit_remote = bit_network;         // session binds a remote resource
  static constexpr unsigned int bit_all = 0xffffffff;
  static constexpr unsigned int type_undef = 0u;
  static constexpr unsigned int type_local = bit_local;
  static constexpr unsigned int type_remote = bit_local;
  friend class service;
  friend class reactor;

  public:
          session(unsigned int) noexcept;
          session(unsigned int, int, int) noexcept;
          session(const session&) noexcept = delete;
          session(session&&) noexcept = delete;
  virtual ~session();
          bool     attach(service*) noexcept;
          bool     detach(service*) noexcept;
  virtual service* get_service_ptr(int) noexcept override;
  virtual int      get_service_count() const noexcept override;
          bool     has_service_flags(unsigned int, unsigned int = bit_all) const noexcept;
          auto     get_service_flags() const noexcept;
          session& operator=(const session&) noexcept = delete;
          session& operator=(session&&) noexcept = delete;
};

/*namespace emc*/ }
#endif
