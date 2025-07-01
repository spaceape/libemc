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
#include "latch.h"

namespace emc {

      latch::latch(bool enable) noexcept:
      latch(0.0f, 0.0f, enable)
{
}

      latch::latch(float value, float threshold, bool enable) noexcept:
      m_value(value),
      m_threshold(threshold),
      m_latch_bit(false),
      m_enable_bit(enable)
{
}

      latch::latch(const latch& copy) noexcept:
      m_value(copy.m_value),
      m_threshold(copy.m_threshold),
      m_latch_bit(copy.m_latch_bit),
      m_enable_bit(copy.m_enable_bit)
{
}

      latch::latch(latch&& copy) noexcept:
      m_value(copy.m_value),
      m_threshold(copy.m_threshold),
      m_latch_bit(copy.m_latch_bit),
      m_enable_bit(copy.m_enable_bit)
{
}

      latch::~latch()
{
}

float latch::get() const noexcept
{
      return m_value;
}

float latch::compare() const noexcept
{
      return m_threshold - m_value;
}

bool  latch::test() const noexcept
{
      if(m_enable_bit) {
          return m_latch_bit;
      } else
          return false;
}

bool  latch::get_latch_bit() const noexcept
{
      return m_latch_bit;
}

bool  latch::set_latch_bit() noexcept
{
      if(m_latch_bit == false) {
          if(m_enable_bit == true) {
              if(m_value >= m_threshold) {
                  m_latch_bit = true;
                  return true;
              }
          }
      }
      return false;
}

void  latch::resume(bool value) noexcept
{
      if(m_enable_bit != value) {
          m_enable_bit = value;
          if(m_enable_bit == false) {
              reset();
          }
      }
}

void  latch::suspend() noexcept
{
      resume(false);
}

void  latch::sync(float dt) noexcept
{
      if(m_enable_bit) {
          m_value += dt;
          set_latch_bit();
      }
}

void  latch::reset() noexcept
{
      m_value = 0.0f;
      m_latch_bit = false;
}

      latch::operator bool() const noexcept
{
      return m_enable_bit;
}

latch& latch::operator=(const latch& rhs) noexcept
{
      m_value = rhs.m_value;
      m_threshold = rhs.m_threshold;
      m_enable_bit = rhs.m_enable_bit;
      m_latch_bit = rhs.m_latch_bit;
      return *this;
}

latch& latch::operator=(latch&& rhs) noexcept
{
      m_value = rhs.m_value;
      m_threshold = rhs.m_threshold;
      m_enable_bit = rhs.m_enable_bit;
      m_latch_bit = rhs.m_latch_bit;
      return *this;
}

/*namespace emc*/ }
