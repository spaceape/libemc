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
#include "uart.h"
#include <emc/error.h>

namespace emc {
namespace transport {

      uart::uart() noexcept:
      emcstage(),
      m_format(codec_format_none),
      m_cache_ptr(nullptr),
      m_cache_size(0),
      m_ready_bit(false)
{
}

      uart::~uart()
{
      emi_cache_dispose();
}

bool  uart::emi_cache_reserve(int size) noexcept
{
      void* l_cache_ptr;
      int   l_cache_size;
      if(size > m_cache_size) {
          l_cache_size = get_round_value(size, global::cache_small_max);
          l_cache_ptr  = realloc(m_cache_ptr, l_cache_size);
          if(l_cache_ptr != nullptr) {
              m_cache_ptr = reinterpret_cast<std::uint8_t*>(l_cache_ptr);
              m_cache_size = l_cache_size;
              return true;
          }
          return false;
      }
      return true;
}

void  uart::emi_cache_dispose() noexcept
{
      if(m_cache_ptr != nullptr) {
          free(m_cache_ptr);
          m_cache_ptr = nullptr;
      }
      m_cache_size = 0;
}

bool  uart::emc_std_resume(emc::gateway*) noexcept
{
      return emi_cache_reserve(global::cache_large_max);
}

int   uart::emc_std_process_packet(int channel, int size, std::uint8_t* data) noexcept
{
      if(size > 0) {
          std::uint8_t* l_forward_data = data;
          int           l_forward_size = size;
          if(m_format == codec_format_base16) {
              l_forward_data = m_cache_ptr;
              l_forward_size = size / 2;
              if(emi_cache_reserve(l_forward_size)) {
                  base16_decode(l_forward_data, data, size);
                  return emc::err_okay;
              } else
                  return emc::err_fail;
          } else
          if(m_format == codec_format_base64) {
              if(size < std::numeric_limits<int>::max() / 6) {
                  l_forward_data = m_cache_ptr;
                  l_forward_size = get_round_value(size * 6 / 8, 8);
                  if(emi_cache_reserve(l_forward_size)) {
                      base64_decode(l_forward_data, data, size);
                      return emc::err_okay;
                  } else
                      return emc::err_fail;
              } else
                  return emc::err_fail;
          } else
          if(m_format != codec_format_none) {
              return emc::err_fail;
          }
          return emc::emcstage::emc_std_process_packet(channel, l_forward_size, l_forward_data);
      }
      return emc::err_okay;
}

int   uart::emc_std_return_packet(int channel, int size, std::uint8_t* data) noexcept
{
      if(m_cache_ptr != nullptr) {
          return emc::err_fail;
      }
      return emc::err_okay;
}

void  uart::emc_std_suspend(emc::gateway*) noexcept
{
      return emi_cache_dispose();
}

/*namespace transport*/ }
/*namespace emc*/ }
