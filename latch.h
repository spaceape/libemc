#ifndef emc_latch_h
#define emc_latch_h
/** 
    Copyright (c) 2024, wicked systems
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
#include <sys/time.h>

namespace emc {

class latch
{
  float   m_value;
  float   m_threshold;
  bool    m_latch_bit;
  bool    m_enable_bit;

  public:
          latch(bool = true) noexcept;
          latch(float, float, bool = true) noexcept;
          latch(const latch&) noexcept;
          latch(latch&&) noexcept;
          ~latch();
          float  get() const noexcept;
          float  compare() const noexcept;
          bool   test() const noexcept;
          bool   get_latch_bit() const noexcept;
          bool   set_latch_bit() noexcept;
          void   resume(bool = true) noexcept;
          void   suspend() noexcept;
          void   update(float) noexcept;
          void   reset() noexcept;
                 operator bool() const noexcept;
          latch& operator=(const latch&) noexcept;
          latch& operator=(latch&&) noexcept;
};

/*namespace emc*/ }
#endif
