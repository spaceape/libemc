#ifndef emc_stage_h
#define emc_stage_h
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
#include "error.h"
#include "event.h"

namespace emc {

class stage
{
  stage*        p_stage_prev;
  stage*        p_stage_next;

  public:
  enum class role {
    undef,
    host,
    user,
    proxy
  };

  protected:
  reactor*      p_owner;
  unsigned int  m_flags;

  protected:
          auto  emc_get_owner() noexcept -> reactor*;
  virtual void  emc_raw_attach(reactor*) noexcept;
  virtual bool  emc_raw_resume(reactor*) noexcept;
  virtual void  emc_raw_join() noexcept;
  virtual void  emc_raw_proto_up(const char*, const char*, unsigned int) noexcept;
  virtual void  emc_raw_recv(std::uint8_t*, std::size_t) noexcept;
  virtual int   emc_raw_feed(std::uint8_t*, std::size_t) noexcept;
  virtual int   emc_raw_send(std::uint8_t*, std::size_t) noexcept;
  virtual void  emc_raw_proto_down() noexcept;
  virtual void  emc_raw_drop() noexcept;
  virtual void  emc_raw_suspend(reactor*) noexcept;
  virtual void  emc_raw_detach(reactor*) noexcept;
  virtual void  emc_raw_sync(float) noexcept;
          void  emc_raw_post(int) noexcept;
  virtual int   emc_raw_post(int, const event_t&) noexcept;
  friend  class reactor;

  public:
          stage(unsigned int = emi_kind_monitor) noexcept;
          stage(const stage&) noexcept = delete;
          stage(stage&&) noexcept = delete;
  virtual ~stage();

          bool         has_owner() const noexcept;
          bool         has_owner(reactor*) const noexcept;
          reactor*     get_owner() noexcept;

          bool         has_type(const char*) const noexcept;
  virtual const char*  get_type() const noexcept;
          bool         has_kind(unsigned int) const noexcept;
          bool         has_kind(unsigned int, unsigned int) const noexcept;
          unsigned int get_kind() const noexcept;

  virtual const char*  get_layer_name(int) const noexcept;
          bool         has_ring_flags(unsigned int) const noexcept;
          unsigned int get_ring_flags() const noexcept;
  virtual void         describe() noexcept;

          void         sync(float) noexcept;

          stage&       operator=(const stage&) noexcept = delete;
          stage&       operator=(stage&&) noexcept = delete;
};

/*namespace emc*/ }
#endif
