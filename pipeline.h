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
  controller*   p_owner;

  protected:
  virtual void  emc_raw_attach(controller*) noexcept;
  virtual void  emc_raw_join() noexcept;
  virtual int   emc_raw_feed(std::uint8_t*, int) noexcept;
  virtual int   emc_raw_send(std::uint8_t*, int) noexcept;
  virtual void  emc_raw_drop() noexcept;
  virtual void  emc_raw_detach(controller*) noexcept;

  protected:
  friend  class controller;

  public:
          rawstage() noexcept;
          rawstage(const rawstage&) noexcept = delete;
          rawstage(rawstage&&) noexcept = delete;
  virtual ~rawstage();
  virtual void      sync(float) noexcept;
          rawstage& operator=(const rawstage&) noexcept = delete;
          rawstage& operator=(rawstage&&) noexcept = delete;
};

/* emcstage
   base class for emc stages
*/
class emcstage
{
  emcstage*     p_stage_prev;
  emcstage*     p_stage_next;

  protected:
  gateway*      p_owner;

  protected:
  virtual void  emc_std_attach(gateway*) noexcept;
  virtual void  emc_std_join() noexcept;
  virtual void  emc_std_connect(const char*, const char*, int) noexcept;
  virtual int   emc_std_process_request(int, const sys::argv&) noexcept;
  virtual int   emc_std_process_response(int, const sys::argv&) noexcept;
  virtual void  emc_std_process_comment(const char*, int) noexcept;
  virtual int   emc_std_process_packet(int, int, std::uint8_t*) noexcept;
  virtual int   emc_std_return_message(const char*, int) noexcept;
  virtual int   emc_std_return_packet(int, int, std::uint8_t*) noexcept;
  virtual void  emc_std_disconnect() noexcept;
  virtual void  emc_std_drop() noexcept;
  virtual void  emc_std_detach(gateway*) noexcept;

  protected:
  friend  class gateway;

  public:
          emcstage() noexcept;
          emcstage(const emcstage&) noexcept = delete;
          emcstage(emcstage&&) noexcept = delete;
  virtual ~emcstage();
  virtual void      sync(float) noexcept;
          emcstage& operator=(const emcstage&) noexcept = delete;
          emcstage& operator=(emcstage&&) noexcept = delete;
};

/*namespace emc*/ }
#endif