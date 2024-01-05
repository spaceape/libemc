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

static const char s_base64_map[256] = {
    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,  62,   0,   0,   0,  63,
   52,  53,  54,  55,  56,  57,  58,  59,   60,  61,   0,   0,   0,   0,   0,   0,

    0,   0,   1,   2,   3,   4,   5,   6,    7,   8,   9,  10,  11,  12,  13,  14,
   15,  16,  17,  18,  19,  20,  21,  22,   23,  24,  25,   0,   0,   0,   0,   0,
    0,  26,  27,  28,  29,  30,  31,  32,   33,  34,  35,  36,  37,  38,  39,  40,
   41,  42,  43,  44,  45,  46,  47,  48,   49,  50,  51,   0,   0,   0,   0,   0,

    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,

    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0
};

namespace emc {
namespace transport {

void  base64_encode(std::uint8_t* dst, std::uint8_t* src, int size) noexcept
{
}

void  base64_decode(std::uint8_t* dst, std::uint8_t* src, int size) noexcept
{
      std::uint8_t  l_digit;
      std::uint8_t* p_dst = dst;
      std::uint8_t* p_src = src;
      std::uint8_t* p_end = src + size;
      while(p_src < p_end) {
          l_digit = s_base64_map[*(p_src++)];
          p_dst[0] = l_digit;
          if(p_src < p_end) {
              p_dst[0] <<= 2;
              l_digit = s_base64_map[*(p_src++)];
              p_dst[0] |= (l_digit & 0b110000) >> 4;
              p_dst[1] = l_digit & 0b001111;
              if(p_src < p_end) {
                  p_dst[1] <<= 4;
                  l_digit = s_base64_map[*(p_src++)];
                  p_dst[1] |= (l_digit & 0x111100) >> 2;
                  p_dst[2] = l_digit & 0x000011;
                  if(p_src < p_end) {
                      p_dst[2] <<= 6;
                      l_digit = s_base64_map[*(p_src++)];
                      p_dst[2] |= l_digit;
                  }
              }
          }
          p_dst += 3;
      }
}

/*namespace transport*/ }
/*namespace emc*/ }