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
#include "timer.h"

namespace emc {

      timer::timer(bool enable) noexcept:
      timer(0.0f, enable)
{
}

      timer::timer(float value, bool enable) noexcept:
      m_value(value),
      m_enable(enable)
{
}

      timer::timer(const timer& copy) noexcept:
      m_value(copy.m_value),
      m_enable(copy.m_enable)
{
}

      timer::timer(timer&& copy) noexcept:
      m_value(copy.m_value),
      m_enable(copy.m_enable)
{
}

      timer::~timer()
{
}

float timer::get() const noexcept
{
      return m_value;
}

float timer::compare(float interval) const noexcept
{
      return interval - m_value;
}

bool  timer::test(float interval) const noexcept
{
      if(m_enable) {
          return compare(interval) <= 0;
      } else
          return false;
}

void  timer::resume(bool value) noexcept
{
      if(m_enable != value) {
          m_enable = value;
          if(m_enable == false) {
              reset();
          }
      }
}

void  timer::suspend() noexcept
{
      resume(false);
}

void  timer::update(float dt) noexcept
{
      if(m_enable) {
          m_value += dt;
      }
}

void  timer::reset() noexcept
{
      m_value = 0.0f;
}

      timer::operator bool() const noexcept
{
      return m_enable;
}

timer& timer::operator=(const timer& rhs) noexcept
{
      m_value = rhs.m_value;
      m_enable = rhs.m_enable;
      return *this;
}

timer& timer::operator=(timer&& rhs) noexcept
{
      m_value = rhs.m_value;
      m_enable = rhs.m_enable;
      return *this;
}

/*namespace emc*/ }
