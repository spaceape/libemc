#ifndef emc_mapper_h
#define emc_mapper_h
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
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR mapperS;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
    EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/
#include "controller.h"
#include <config.h>

namespace emc {

/* device_t, stream_t
   device and stream descriptors
*/
constexpr int   device_name_size = 8;

constexpr char  edm_flag_none = '-';
constexpr char  edm_flag_recv = 'r';
constexpr char  edm_flag_send = 'w';
constexpr char  edm_flag_binary = 'b';
constexpr char  edm_flag_encode_base16 = 'h';
constexpr char  edm_flag_encode_base64 = 's';

constexpr std::uint8_t  edf_none = 0u;
constexpr std::uint8_t  edf_allow_recv  = 0x01;
constexpr std::uint8_t  edf_allow_send  = 0x02;
constexpr std::uint8_t  edf_allow_seek  = 0x04;
constexpr std::uint8_t  edf_allow_sync  = 0x08;
constexpr std::uint8_t  edf_mode_binary = 0x80;

constexpr std::uint8_t edt_none = 0u;

struct device_t
{
  std::uint8_t    ed_type;
  std::uint8_t    ed_flags;
  std::int8_t     ed_instance_count;
  std::int8_t     ed_instance_limit;
  const char*     ed_name;
};

constexpr std::uint8_t  esf_none = 0u;
constexpr std::uint8_t  esf_mode_recv = edf_allow_recv;
constexpr std::uint8_t  esf_mode_send = edf_allow_send;
constexpr std::uint8_t  esf_mode_seek = edf_allow_seek;
constexpr std::uint8_t  esf_mode_sync = edf_allow_sync;
constexpr std::uint8_t  esf_encoding_base16 = 0x10;
constexpr std::uint8_t  esf_encoding_base64 = 0x40;
constexpr std::uint8_t  esf_encoding_binary = edf_mode_binary;
constexpr std::uint8_t  esf_encoding_any = esf_encoding_base16 | esf_encoding_base64;

struct stream_t
{
  std::uint8_t    es_type;
  std::int8_t     es_device;
  std::int8_t     es_channel;
  std::uint8_t    es_flags;
  int             es_rate;
  int             es_offset;
  int             es_size;
};

/* mapper
   base class for mappers - special controllers that implement the `dev` layer
*/
class mapper: public emc::controller
{
  emc::gateway*   p_gateway;
  std::uint8_t*   m_cache_data;
  int             m_cache_size;
  emc::stream_t   m_stream_list[16];
  int             m_search_index;
  int             m_stream_count;

  protected:
  emc::device_t   m_device_list[16];
  int             m_device_count;

  private:
          auto    mpi_cache_reserve(int) noexcept -> std::uint8_t*;
          void    mpi_cache_dispose() noexcept;

  protected:
          auto    mpi_find_channel() noexcept -> std::int8_t;
          bool    mpi_acquire_channel(int, stream_t*) noexcept;
          bool    mpi_release_channel(int, stream_t*) noexcept;
          auto    mpi_get_device_ptr(int) noexcept -> emc::device_t*;
          auto    mpi_find_device_ptr(const char*) noexcept -> emc::device_t*;
          int     mpi_find_device_index(const char*) noexcept;
          auto    mpi_find_stream() noexcept -> emc::stream_t*;

  virtual auto    mpi_get_type_string(std::uint8_t) noexcept -> const char*;
  virtual bool    mpi_get_device_info(emc::device_t*, char*, int) noexcept;
  virtual bool    mpi_get_stream_info(emc::stream_t*, char*, int) noexcept;
  virtual int     mpi_open_stream(emc::stream_t*, emc::device_t*, const sys::argv&) noexcept;
  virtual void    mpi_recv(emc::stream_t*, std::uint8_t*, int) noexcept;
  virtual int     mpi_close_stream(emc::stream_t*) noexcept;

          void    mpi_send_support_event(emc::device_t*, char) noexcept;
          void    mpi_send_channel_event(emc::stream_t*, char) noexcept;
          void    mpi_send_support_response(int, char) noexcept;
          void    mpi_send_support_response() noexcept;
          void    mpi_send_eol() noexcept;

  virtual void    emc_std_attach(gateway*) noexcept override;
  virtual int     emc_std_process_request(int, const sys::argv&) noexcept override;
  virtual int     emc_std_process_response(int, const sys::argv&) noexcept override;
  virtual int     emc_std_process_packet(int, int, std::uint8_t*) noexcept override;
  virtual void    emc_std_detach(gateway*) noexcept override;
  virtual void    emc_std_sync(float) noexcept override;

  public:
          mapper() noexcept;
          mapper(const mapper&) noexcept = delete;
          mapper(mapper&&) noexcept = delete;
          ~mapper();
  virtual void    describe() noexcept override final;
          mapper& operator=(const mapper&) noexcept = delete;
          mapper& operator=(mapper&&) noexcept = delete;
};

/*namespace emc*/ }
#endif
