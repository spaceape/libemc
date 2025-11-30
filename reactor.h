#ifndef emc_reactor_h
#define emc_reactor_h
/**
    Copyright (c) 2025, wicked systems
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
#include "stage.h"

namespace emc {

/* reactor
   - sys_*: core functions;
   - emc_raw_*: callbacks, same pattern as hte stage::emc_raw_* functions.
*/
class reactor
{
  stage*        p_stage_head;
  stage*        p_stage_tail;

  protected:
  stage*        p_recv_stage;     // input stage
  stage*        p_core_stage;     // protocol stage
  std::uint8_t  m_enable_events;
  std::uint8_t  m_record_events;

  protected:
  bool          m_resume_bit;
  bool          m_join_bit;
  bool          m_open_bit;

  private:
  bool          m_record_enable;

  private:
          void  sys_attach(stage*) noexcept;
          bool  sys_resume_all() noexcept;
          void  sys_join_all() noexcept;
          void  sys_open_all() noexcept;
          void  sys_close(stage*) noexcept;
          void  sys_close_all() noexcept;
          void  sys_drop(stage*) noexcept;
          void  sys_drop_all() noexcept;
          void  sys_suspend(stage*) noexcept;
          void  sys_suspend_all() noexcept;
          void  sys_suspend_all(stage*) noexcept;
          void  sys_detach(stage*) noexcept;
          void  sys_detach_all() noexcept;
          void  sys_sync_all(float) noexcept;

          void  sys_suspend_events(std::uint8_t&, std::uint8_t) noexcept;
          void  sys_restore_events(std::uint8_t&) noexcept;
          void  sys_record_events() noexcept;
          void  sys_delete_events() noexcept;

  protected:
  virtual bool  emc_raw_resume() noexcept;
          int   emc_raw_recv(int, std::uint8_t*, std::size_t) noexcept;
  virtual bool  emc_raw_suspend() noexcept;
  virtual void  emc_raw_sync(float) noexcept;
  virtual int   emc_raw_event(event, const event_info_t&) noexcept;

          bool  pod_resume() noexcept;
          bool  pod_attach_stage(stage*) noexcept;
          bool  pod_detach_stage(stage*) noexcept;
          bool  pod_suspend(bool = true) noexcept;

  friend class stage;
  public:
          reactor() noexcept;
          reactor(const reactor&) noexcept = delete;
          reactor(reactor&&) noexcept = delete;
  virtual ~reactor();

  virtual void      feed(int) noexcept;
  virtual void      hup(int) noexcept;
          int       post(event, const event_info_t&) noexcept;
          void      sync(float) noexcept;

          reactor&  operator=(const reactor&) noexcept = delete;
          reactor&  operator=(reactor&&) noexcept = delete;
};

/*namespace emc*/ }
#endif
