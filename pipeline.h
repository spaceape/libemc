#ifndef emc_pipeline_h
#define emc_pipeline_h
/** 
    Copyright (c) 2023, wicked systems
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
#include <sys/fmt.h>
#include <sys/argv.h>

namespace emc {

/* rawstage
   base class for raw stages
*/
class rawstage
{
  rawstage*     p_stage_prev;
  rawstage*     p_stage_next;

  protected:
  reactor*      p_owner;

  protected:
  virtual void  emc_raw_attach(reactor*) noexcept;
  virtual bool  emc_raw_resume(reactor*) noexcept;
  virtual void  emc_raw_join() noexcept;
  virtual void  emc_raw_recv(std::uint8_t*, int) noexcept;
  virtual int   emc_raw_feed(std::uint8_t*, int) noexcept;
  virtual int   emc_raw_send(std::uint8_t*, int) noexcept;
  virtual void  emc_raw_drop() noexcept;
  virtual void  emc_raw_suspend(reactor*) noexcept;
  virtual int   emc_raw_event(int, void*) noexcept;
  virtual void  emc_raw_detach(reactor*) noexcept;
  virtual void  emc_raw_sync(float) noexcept;

  protected:
  friend  class reactor;

  public:
          rawstage() noexcept;
          rawstage(const rawstage&) noexcept = delete;
          rawstage(rawstage&&) noexcept = delete;
  virtual ~rawstage();
  virtual const char* get_type() const noexcept;
          rawstage&   operator=(const rawstage&) noexcept = delete;
          rawstage&   operator=(rawstage&&) noexcept = delete;
};

/* emcstage
   base class for emc stages
*/
class emcstage
{
  emcstage*     p_stage_prev;
  emcstage*     p_stage_next;

  public:
  static constexpr int layer_state_disabled = 0;
  static constexpr int layer_state_enabled = 1;

  protected:
  gateway*      p_owner;

  protected:
          void  emc_emit(char) noexcept;
          void  emc_emit(int, const char*) noexcept;
  virtual void  emc_std_attach(gateway*) noexcept;
  virtual bool  emc_std_resume(gateway*) noexcept;
  virtual void  emc_std_join() noexcept;
  virtual void  emc_std_connect(const char*, const char*, int) noexcept;
  virtual void  emc_std_process_message(const char*, int) noexcept;
  virtual void  emc_std_process_error(const char*, int) noexcept;
  virtual int   emc_std_process_request(int, const sys::argv&) noexcept;
  virtual int   emc_std_process_response(int, const sys::argv&) noexcept;
  virtual void  emc_std_process_comment(const char*, int) noexcept;
  virtual int   emc_std_process_packet(int, int, std::uint8_t*) noexcept;
  virtual int   emc_std_return_message(const char*, int) noexcept;
  virtual int   emc_std_return_packet(int, int, std::uint8_t*) noexcept;
  virtual void  emc_std_disconnect() noexcept;
  virtual void  emc_std_drop() noexcept;
  virtual void  emc_std_suspend(gateway*) noexcept;
  virtual int   emc_std_event(int, void*) noexcept;
  virtual void  emc_std_detach(gateway*) noexcept;
          int   emc_error(int, const char*, ...) noexcept;
  virtual void  emc_std_sync(float) noexcept;

  protected:
  friend  class gateway;

  public:
          emcstage() noexcept;
          emcstage(const emcstage&) noexcept = delete;
          emcstage(emcstage&&) noexcept = delete;
  virtual ~emcstage();
  virtual void         describe() noexcept;
  virtual const char*  get_layer_name(int) const noexcept;
  virtual int          get_layer_state(int) const noexcept;
          emcstage&    operator=(const emcstage&) noexcept = delete;
          emcstage&    operator=(emcstage&&) noexcept = delete;
};

/*namespace emc*/ }
#endif
