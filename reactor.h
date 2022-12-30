#ifndef emc_reactor_h
#define emc_reactor_h
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
#include "host.h"
#include "gateway.h"
#include <sys.h>
#include <memory>
#include <string>
#include <mmi.h>
#include <mmi/resource.h>
#include <mmi/bank.h>
#include <mmi/flat_list.h>
#include <mmi/flat_map.h>
#include "session.h"

namespace emc {

/* reactor
 * integrated base class for an EMC server
*/
class reactor
{
  int  m_poll_descriptor;
  int  m_poll_count;

  private:
  mmi::flat_list<host>         m_interface_list;
  mmi::flat_map<int, host*>    m_interface_by_descriptor;
  mmi::flat_map<int, session*> m_session_by_descriptor;

  int  m_interface_count;
  int  m_interface_limit;
  int  m_session_count;
  int  m_session_limit;

  bool m_ready_bit;
  bool m_resume_bit;

  private:
          bool      ctl_desc_attach(int) noexcept;
          bool      ctl_desc_remove(int) noexcept;
          host*     ctl_interface_find(int) noexcept;
          session*  ctl_session_spawn(int, host*) noexcept;
          bool      ctl_session_attach(int, session*) noexcept;
          void      ctl_session_drop(int) noexcept;
          session*  ctl_session_find(int) noexcept;
          void      ctl_session_feed(session*) noexcept;
          void      ctl_session_suspend(int, session*) noexcept;

  protected:
  virtual session*  emc_spawn_session(host*, int) noexcept;
  virtual bool      emc_suspend_session(session*) noexcept;
  virtual void      emc_sync(const sys::time_t&) noexcept;

  protected:
          bool      emc_attach_session(session*) noexcept;
          void      emc_detach_session(session*) noexcept;
          host*     emc_attach_interface(host::type, const std::string&) noexcept;
          void      emc_detach_interface(host*) noexcept;

  public:
          reactor() noexcept;
          reactor(const reactor&) noexcept = delete;
          reactor(reactor&&) noexcept = delete;
  virtual ~reactor();
          bool     resume() noexcept;
  virtual const char* get_machine_name() const noexcept;
  virtual const char* get_machine_type() const noexcept;
          bool     suspend() noexcept;
          void     sync(const sys::time_t&, const sys::delay_t&) noexcept;
          reactor& operator=(const reactor&) noexcept = delete;
          reactor& operator=(reactor&&) noexcept = delete;
};

/*namespace emc*/ }
#endif
