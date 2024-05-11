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

static const char s_base64_encode_map[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',  '4', '5', '6', '7', '8', '9', '+', '/'
};

static const char s_base64_decode_map[256] = {
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

std::size_t base64_encode(std::uint8_t* dst, std::uint8_t* src, std::size_t size) noexcept
{
      unsigned int  i_cvt;
      std::uint8_t* p_dst = dst;
      std::uint8_t* p_src = src;
      int           l_rem_size = size % 3;
      int           l_cpt_size = size - l_rem_size;
      std::uint8_t* p_end = src + l_cpt_size;
      while(p_src < p_end) {
          i_cvt = *(p_src++) << 16;
          i_cvt |= *(p_src++) << 8;
          i_cvt |= *(p_src++);
          *(p_dst++) = s_base64_encode_map[(i_cvt & 0b11111100'00000000'00000000) >> 18];
          *(p_dst++) = s_base64_encode_map[(i_cvt & 0b00000011'11110000'00000000) >> 12];
          *(p_dst++) = s_base64_encode_map[(i_cvt & 0b00000000'00001111'11000000) >> 6];
          *(p_dst++) = s_base64_encode_map[(i_cvt & 0b00000000'00000000'00111111)];
      }
      if(l_rem_size > 0) {
          i_cvt = *(p_src++) << 16;
          *(p_dst++) = s_base64_encode_map[(i_cvt & 0b11111100'00000000'00000000) >> 18];
          if(l_rem_size > 1) {
              i_cvt |= *(p_src++) << 8;
              *(p_dst++) = s_base64_encode_map[(i_cvt & 0b00000011'11110000'00000000) >> 12];
              *(p_dst++) = s_base64_encode_map[(i_cvt & 0b00000000'00001111'00000000) >> 6];
          }
          else {
              *(p_dst++) = s_base64_encode_map[(i_cvt & 0b00000011'00000000'00000000) >> 12];
              *(p_dst++) = '=';
          }
          *(p_dst++) = '=';
      }
      return p_dst - dst;
}

std::size_t base64_decode(std::uint8_t* dst, std::uint8_t* src, std::size_t size) noexcept
{
      unsigned int  i_cvt;
      std::uint8_t* p_dst = dst;
      std::uint8_t* p_src = src;
      int           l_rem_size = size % 4;
      int           l_cpt_size = size - l_rem_size;
      std::uint8_t* p_end = src + l_cpt_size;
      while(p_src < p_end) {
          i_cvt = s_base64_decode_map[*(p_src++)] << 18;
          i_cvt |= s_base64_decode_map[*(p_src++)] << 12;
          i_cvt |= s_base64_decode_map[*(p_src++)] << 6;
          i_cvt |= s_base64_decode_map[*(p_src++)];
          *(p_dst++) = (i_cvt & 0xff0000) >> 16;
          *(p_dst++) = (i_cvt & 0x00ff00) >> 8;
          *(p_dst++) = (i_cvt & 0x0000ff);
      }
      if(l_rem_size > 0) {
          i_cvt = s_base64_decode_map[*(p_src++)] << 18;
          if(l_rem_size > 1) {
              i_cvt |= s_base64_decode_map[*(p_src++)] << 12;
              *(p_dst++) = (i_cvt & 0b11111111'00000000'00000000) >> 16;
              if(l_rem_size > 2) {
                  i_cvt |= s_base64_decode_map[*(p_src++)] << 6;
                  *(p_dst++) = (i_cvt & 0b00000000'11111111'00000000) >> 8;
                  *(p_dst++) = (i_cvt & 0b00000000'00000000'11000000);
              }
              else {
                  *(p_dst++) = (i_cvt & 0b00000000'11110000'00000000) >> 8;
              }
          }
          else {
              *(p_dst++) = (i_cvt & 0b11111100'00000000'00000000) >> 16;
          }
      }
      return p_dst - dst;
}

/*namespace transport*/ }
/*namespace emc*/ }
