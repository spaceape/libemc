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
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR controllerS;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
    EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/
#include "emc.h"
#include "pipeline.h"

namespace emc {

/* controller
   base class for EMC controller stages
*/
class controller: public emcstage
{
  protected:
  inline  void  emc_put() noexcept {
  }

  template<typename... Args>
  inline  void  emc_put(char c, Args&&... next) noexcept {
          emc_emit(c);
          emc_put(std::forward<Args>(next)...);
  }

  template<typename... Args>
  inline  void  emc_put(const char* text, Args&&... next) noexcept {
          emc_emit(0, text);
          emc_put(std::forward<Args>(next)...);
  }

  template<typename... Args>
  inline  void  emc_put(const fmt::d& value, Args&&... next) noexcept {
          emc_emit(0, value);
          emc_put(std::forward<Args>(next)...);
  }

  template<typename... Args>
  inline  void  emc_put(const fmt::x& value, Args&&... next) noexcept {
          emc_emit(0, value);
          emc_put(std::forward<Args>(next)...);
  }

  template<typename... Args>
  inline  void  emc_put(const fmt::X& value, Args&&... next) noexcept {
          emc_emit(0, value);
          emc_put(std::forward<Args>(next)...);
  }

  template<typename... Args>
  inline  void  emc_put(const fmt::f& value, Args&&... next) noexcept {
          emc_emit(0, value);
          emc_put(std::forward<Args>(next)...);
  }

  public:
          controller() noexcept;
          controller(const controller&) noexcept = delete;
          controller(controller&&) noexcept = delete;
          ~controller();
          controller&  operator=(const controller&) noexcept = delete;
          controller&  operator=(controller&&) noexcept = delete;
};

/*namespace emc*/ }
#endif
