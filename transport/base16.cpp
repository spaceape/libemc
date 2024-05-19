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
#include <emc.h>

static const char s_base16_encode_map[256] = {
    '0', '1', '2', '3', '4', '5', '6', '7',  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

static const char s_base16_decode_map[256] = {
    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   1,   2,   3,   4,   5,   6,   7,    8,   9,   0,   0,   0,   0,   0,   0,

    0,  10,  11,  12,  13,  14,  15,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,  10,  11,  12,  13,  14,  15,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0
};

namespace emc {
namespace transport {

std::size_t base16_encode(std::uint8_t* dst, const std::uint8_t* src, std::size_t size) noexcept
{
      auto p_dst = dst;
      auto p_src = src;
      auto p_end = src + size;
      int  hex;
      while(p_src < p_end) {
          hex = (*p_src & 0xf0) >> 4;
          *(p_dst++) = s_base16_encode_map[hex];
          hex = (*p_src & 0x0f);
          *(p_dst++) = s_base16_encode_map[hex];
          p_src++;
      }
      return p_dst - dst;
}

std::size_t base16_encode(std::uint8_t* dst, const char* src, std::size_t size) noexcept
{
      return base16_encode(dst, reinterpret_cast<const std::uint8_t*>(src), size);
}

std::size_t base16_decode(std::uint8_t* dst, const std::uint8_t* src, std::size_t size) noexcept
{
      auto p_dst = dst;
      auto p_src = src;
      auto p_end = src + (size & (~1));
      while(p_src < p_end) {
          *p_dst = s_base16_decode_map[*(p_src++)];
          *p_dst <<= 4;
          *p_dst |= s_base16_decode_map[*(p_src++)];
          p_dst++;
      }
      if(size & 1) {
          *p_dst = s_base16_decode_map[*(p_src++)];
          p_dst++;
      }
      return p_dst - dst;
}

std::size_t base16_decode(std::uint8_t* dst, const char* src, std::size_t size) noexcept
{
      return base16_decode(dst, reinterpret_cast<const std::uint8_t*>(src), size);
}

/*namespace transport*/ }
/*namespace emc*/ }