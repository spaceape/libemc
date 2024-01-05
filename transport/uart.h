#ifndef emc_transport_uart_h
#define emc_transport_uart_h
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
#include <emc/pipeline.h>

namespace emc {
namespace transport {

/* uart
   Transport stage which encodes/decodes binary data in packets
*/
class uart: public emc::emcstage
{
  int             m_format;
  std::uint8_t*   m_cache_ptr;
  int             m_cache_size;
  bool            m_ready_bit;

  public:
  static constexpr int codec_format_none = 0;
  static constexpr int codec_format_base16 = 1;
  static constexpr int codec_format_base64 = 2;
  protected:
          bool    emi_cache_reserve(int) noexcept;
          void    emi_cache_dispose() noexcept;
  virtual bool    emc_std_resume(emc::gateway*) noexcept override;
  virtual int     emc_std_process_packet(int, int, std::uint8_t*) noexcept override;
  virtual int     emc_std_return_packet(int, int, std::uint8_t*) noexcept override;
  virtual void    emc_std_suspend(emc::gateway*) noexcept override;

  public:
          uart() noexcept;
          uart(const uart&) noexcept = delete;
          uart(uart&&) noexcept = delete;
  virtual ~uart();
          uart&   operator=(const uart&) noexcept = delete;
          uart&   operator=(uart&&) noexcept = delete;
};

/*namespace transport*/ }
/*namespace emc*/ }
#endif
