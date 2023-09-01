#ifndef emc_controller_h
#define emc_controller_h
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
#include "pipeline.h"
#include "error.h"

namespace emc {

/* controller
   base class for a raw pipeline
*/
class controller
{
  rawstage*     p_stage_head;
  rawstage*     p_stage_tail;
  int           m_stage_count;

  public:
  enum class role {
    undef,
    host,
    user
  };

  private:
  role          m_role;

  protected:
  virtual void  ems_stage_attach(int, rawstage*) noexcept;
          void  emc_join() noexcept;
          void  emc_feed(std::uint8_t*, int) noexcept;
  virtual int   emc_recv(std::uint8_t*, int) noexcept;
          void  emc_drop() noexcept;
  virtual void  ems_stage_detach(int, rawstage*) noexcept;

  friend  class rawstage;
  public:
          controller(role) noexcept;
          controller(const controller&) noexcept = delete;
          controller(controller&&) noexcept = delete;
  virtual ~controller();
          bool         attach(rawstage*) noexcept;
          bool         detach(rawstage*) noexcept;
          bool         has_role(role) const noexcept;
          role         get_role() const noexcept;
  virtual auto         get_system_name() noexcept -> const char*;
  virtual auto         get_system_type() noexcept -> const char*;
          void         sync(float) noexcept;
          controller&  operator=(const controller&) noexcept = delete;
          controller&  operator=(controller&&) noexcept = delete;
};

/*namespace emc*/ }
#endif
