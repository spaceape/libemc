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
#include "mapper.h"
#include <emc/protocol.h>
#include <emc/error.h>

static emc::stream_t* s_channel_map[128] = {
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,

    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
};

namespace emc {

      mapper::mapper() noexcept:
      controller(),
      m_search_index(chid_min),
      m_stream_count(0),
      m_device_count(0)
{
}

      mapper::~mapper()
{
}

auto  mapper::mpi_find_channel() noexcept -> std::int8_t
{
      int  i_search_wrap = m_search_index;
      int  i_search_index = m_search_index;
      // find the first unused channel id, starting from the set index
      while(i_search_index < chid_max) {
          if(s_channel_map[i_search_index] == nullptr) {
              return i_search_index;
          }
          i_search_index++;
      }
      // did not find an unused channel yet, wrap the search around if possible
      if(i_search_wrap > chid_min) {
          i_search_index = chid_min;
          while(i_search_index < i_search_wrap) {
              if(s_channel_map[i_search_index] == nullptr) {
                  return i_search_index;
              }
              i_search_index++;
          }
      }
      return chid_none;
}

bool  mapper::mpi_acquire_channel(int index, stream_t* stream) noexcept
{
      m_search_index = index + 1;
      if(s_channel_map[index] == nullptr) {
          s_channel_map[index] = stream;
          return true;
      }
      return false;
}

bool  mapper::mpi_release_channel(int index, stream_t* stream) noexcept
{
      if(s_channel_map[index] == stream) {
          if(m_search_index > index) {
              m_search_index = index;
          }
          s_channel_map[index] = nullptr;
          return true;
      }
      return false;
}

auto  mapper::mpi_get_device_ptr(int index) noexcept -> emc::device_t*
{
      return std::addressof(m_device_list[index]);
}

int   mapper::mpi_find_device_index(const char* name) noexcept
{
      int i_device = 0;
      if(name != nullptr) {
          while(i_device < m_device_count) {
              if(std::strncmp(name, m_device_list[i_device].ed_name, emc::device_name_size) == 0) {
                  return i_device;
              }
              ++i_device;
          }
      }
      return -1;
}

auto  mapper::mpi_find_device_ptr(const char* name) noexcept -> emc::device_t*
{
      int i_device = mpi_find_device_index(name);
      if(i_device >= 0) {
          return mpi_get_device_ptr(i_device);
      }
      return nullptr;
}

auto  mapper::mpi_find_stream() noexcept -> emc::stream_t*
{
      emc::stream_t* p_stream;
      int            i_stream = m_stream_count;
      // get the next stream from the list, in the limit of the available slots
      if(i_stream < stream_count_max) {
          p_stream = std::addressof(m_stream_list[i_stream]);
          p_stream->es_type = edt_none;
          p_stream->es_device = -1;
          p_stream->es_channel = -1;
          p_stream->es_flags = esf_none;
          p_stream->es_rate = 0;
          p_stream->es_offset = 0;
          p_stream->es_size = 0;
          m_stream_count++;
          return p_stream;
      }
      // if already maxxed out on slots then try to find an unused slot
      i_stream = m_stream_count - 1;
      while(i_stream >= 0) {
          p_stream = std::addressof(m_stream_list[i_stream]);
          if(p_stream->es_type == edt_none) {
              p_stream->es_device = -1;
              p_stream->es_channel = -1;
              p_stream->es_flags = esf_none;
              p_stream->es_rate = 0;
              p_stream->es_offset = 0;
              p_stream->es_size = 0;
              return p_stream;
          }
          i_stream--;
      }
      return nullptr;
}

auto  mapper::mpi_get_type_string(std::uint8_t type) noexcept -> const char*
{
      switch(type) {
          case edt_none:
              return "none";
          default:
              return nullptr;
      }
}

bool  mapper::mpi_get_device_info(emc::device_t*, char*, int) noexcept
{
      return false;
}

bool  mapper::mpi_get_stream_info(emc::stream_t*, char*, int) noexcept
{
      return false;
}

int   mapper::mpi_open_stream(stream_t*, device_t*, const sys::argv&) noexcept
{
      return emc::err_fail;
}

int   mapper::mpi_close_stream(stream_t*) noexcept
{
      return emc::err_fail;
}

void  mapper::mpi_send_support_event(emc::device_t* device, char event_tag) noexcept
{
      const char*  l_device_name = device->ed_name;
      const char*  l_device_type;
      char         l_device_info[64];
      char         l_device_flags[8];
      int          l_event_tag = event_tag;
      if(device->ed_type != edt_none) {
          // assign a human readable string for the device type
          l_device_type = mpi_get_type_string(device->ed_type);
          emc_put(
              emc::emc_tag_response,
              emc::emc_response_support,
              l_event_tag,
              SPC, fmt::s(l_device_name, device_name_size, SPC)
          );
          if(l_event_tag == emc::emc_enable_tag) {
              // mode flag specifying whether the device has a time base or not
              l_device_flags[0] = '-';
              // mode flag specifying whether the device can be read from or not
              if(device->ed_flags & emc::edf_allow_recv) {
                  l_device_flags[1] = 'r';
              } else
                  l_device_flags[1] = '-';
              // mode flag specifying whether the device can be written to or not
              if(device->ed_flags & emc::edf_allow_recv) {
                  l_device_flags[2] = 'w';
              } else
                  l_device_flags[2] = '-';
              // mode flag for binary vs. ascii mode; useful over UART or other transport protocols which do not allow binary
              // data to be transferred directly
              if(device->ed_flags & emc::edf_mode_binary) {
                  l_device_flags[3] = 'b';
              } else
                  l_device_flags[3] = '-';
              l_device_flags[4] = 0;
              // write out the basic device information
              emc_put(
                  SPC, l_device_flags,
                  SPC, l_device_type
              );
              // write out type-specific information for the device
              if(mpi_get_device_info(device, l_device_info, sizeof(l_device_info))) {
                  emc_put(SPC);
                  emc_put(l_device_info);
              }
          }
          // terminate the device line
          emc_put(EOL);
      }
}

void  mapper::mpi_send_channel_event(emc::stream_t* stream, char event_tag) noexcept
{
      char           l_stream_info[64];
      char           l_stream_flags[8];
      int            l_event_tag = event_tag;
      if(stream->es_type != edt_none) {
          // assign a human readable string for the device type
          emc::device_t* p_device      = mpi_get_device_ptr(stream->es_device);
          const char*    l_device_name = p_device->ed_name;
          const char*    l_stream_type = mpi_get_type_string(stream->es_type);
          emc_put(
              emc::emc_tag_response,
              emc::emc_response_channel,
              l_event_tag,
              SPC, fmt::X(stream->es_channel, 2)
          );
          if(l_event_tag == emc::emc_enable_tag) {
              // mode flag specifying whether the device has a time base or not
              l_stream_flags[0] = '-';
              // mode flag specifying whether the stream can be read from or not
              if(stream->es_flags & emc::edf_allow_recv) {
                  l_stream_flags[1] = 'r';
              } else
                  l_stream_flags[1] = '-';
              // mode flag specifying whether the stream can be written to or not
              if(stream->es_flags & emc::edf_allow_recv) {
                  l_stream_flags[2] = 'w';
              } else
                  l_stream_flags[2] = '-';
              // mode flag for binary vs. ascii mode; useful over UART or other transport protocols which do not allow binary
              // data to be transferred directly
              if(stream->es_flags & emc::edf_mode_binary) {
                  l_stream_flags[3] = 'b';
              } else
                  l_stream_flags[3] = '-';
              l_stream_flags[4] = 0;
              // write out the basic device information
              emc_put(
                  SPC, l_device_name,
                  SPC, l_stream_flags,
                  SPC, l_stream_type
              );
              // write out type-specific information for the device
              if(mpi_get_stream_info(stream, l_stream_info, sizeof(l_stream_info))) {
                  emc_put(SPC);
                  emc_put(l_stream_info);
              }
          }
          // terminate the device line
          emc_put(EOL);
      }
}

void  mapper::mpi_send_support_response(int index, char event_tag) noexcept
{
      mpi_send_support_event(mpi_get_device_ptr(index), event_tag);
}

void  mapper::mpi_send_support_response() noexcept
{
      int i_device = 0;
      while(i_device < m_device_count) {
          mpi_send_support_response(i_device, emc::emc_enable_tag);
          ++i_device;
      }
}

int   mapper::emc_std_process_request(int argc, const sys::argv& argv) noexcept
{
      int l_rc;
      if(argv[0].has_text("support")) {
          if(argc == 1) {
              mpi_send_support_response();
              l_rc = err_okay;
          } else
              l_rc = emc::err_no_request;
      } else
      if(argv[0].has_text("describe")) {
          if(argc == 1) {
              mpi_send_support_response();
              l_rc = err_okay;
          } else
          if(argc == 2) {
              const char*  l_device_name  = argv[1].get_text();
              int          l_device_index = mpi_find_device_index(l_device_name);
              if(l_device_index >= 0) {
                  mpi_send_support_response(l_device_index, emc::emc_enable_tag);
                  l_rc = emc::err_okay;
              } else
                  l_rc = emc::err_no_request;
          } else
              l_rc = emc::err_no_request;
      } else
      if(argv[0].has_text("ctl")) {
          if(argc == 1) {
              l_rc = emc::err_fail;
          } else
              l_rc = emc::err_parse;
      } else
      if(argv[0].has_text("o")) {
          if(argc >= 3) {
              int  l_channel_index = argv[1].get_hex_int();
              if(l_channel_index == 0) {
                  if(argv[1].has_text("*") ||
                      argv[1].has_text("0")) {
                      l_channel_index = mpi_find_channel();
                      if(l_channel_index == chid_none) {
                          l_rc = emc_error(emc::err_fail, "Failed to allocate channel ID for request");
                      }
                  }
              }
              if((l_channel_index >= chid_min) &&
                  (l_channel_index <= chid_max)) {
                  const char* l_device_name  = argv[2].get_text();
                  int         l_device_index = mpi_find_device_index(l_device_name);
                  if(l_device_index >= 0) {
                      emc::device_t* p_device = mpi_get_device_ptr(l_device_index);
                      if(p_device->ed_type != edt_none) {
                          if((p_device->ed_instance_limit <= 0) ||
                              (p_device->ed_instance_count < p_device->ed_instance_limit)) {
                              emc::stream_t* p_stream = mpi_find_stream();
                              if(p_stream != nullptr) {
                                  // assign the channel id to the stream
                                  if(mpi_acquire_channel(l_channel_index, p_stream)) {
                                      p_stream->es_type = p_device->ed_type;
                                      p_stream->es_device = l_device_index;
                                      p_stream->es_channel = l_channel_index;
                                      p_stream->es_flags = p_device->ed_flags;
                                      l_rc = mpi_open_stream(p_stream, p_device, argv);
                                      if(l_rc == emc::err_okay) {
                                          // send success response
                                          mpi_send_channel_event(p_stream, emc::emc_enable_tag);
                                          // update the mapper properties
                                          if(p_device->ed_instance_limit > 0) {
                                              p_device->ed_instance_count++;
                                              if(p_device->ed_instance_count == p_device->ed_instance_limit) {
                                                  mpi_send_support_event(p_device, emc::emc_disable_tag);
                                              }
                                          }
                                      } else
                                      if(l_rc != emc::err_okay) {
                                          // release the previously reserved resources
                                          p_stream->es_type = edt_none;
                                          p_stream->es_device = l_device_index;
                                          p_stream->es_channel = l_channel_index;
                                          mpi_release_channel(l_channel_index, p_stream);
                                      }
                                  } else
                                      l_rc = emc_error(emc::err_fail, "Failed to acquire channel `%.2d` for device \"%s\"", l_channel_index, p_device->ed_name);
                              } else
                                  l_rc = emc_error(emc::err_fail, "Failed to allocate stream for device \"%s\"", p_device->ed_name);
                          } else
                              l_rc = emc_error(emc::err_fail, "Instance count exceeded for device \"%s\"", p_device->ed_name);
                      } else
                          l_rc = emc_error(emc::err_fail, "Invalid device: \"%s\"", p_device->ed_name);
                  } else
                      l_rc = emc::err_no_request;
              } else
                  l_rc = emc_error(emc::err_bad_request, "Invalid channel ID in request");
          } else
              l_rc = emc::err_no_request;
      } else
      if(argv[0].has_text("x")) {
          if(argc == 2) {
              int l_channel_index = argv[1].get_hex_int();
              if((l_channel_index >= chid_min) &&
                  (l_channel_index <= chid_max)) {
                  // read the channel map in an attempt to find the stream attached to the specified channel
                  emc::stream_t* p_stream = s_channel_map[l_channel_index];
                  if(p_stream != nullptr) {
                      // check if the stream belongs to our list
                      int  l_stream_index = p_stream - std::addressof(m_stream_list[0]);
                      if((l_stream_index >= 0) &&
                          (l_stream_index < m_stream_count)) {
                          if(p_stream->es_type != edt_none) {
                              emc::device_t*  p_device = mpi_get_device_ptr(p_stream->es_device);
                              if(p_device != nullptr) {
                                  // send out the release event
                                  mpi_send_channel_event(p_stream, emc::emc_disable_tag);
                                  // release the channel
                                  mpi_release_channel(l_channel_index, p_stream);
                                  // release the stream object
                                  p_stream->es_type = edt_none;
                                  p_stream->es_device = -1;
                                  p_stream->es_channel = -1;
                                  p_stream->es_flags = esf_none;
                                  // restore the device instance count
                                  if(p_device->ed_instance_limit > 0) {
                                      if(p_device->ed_instance_count == p_device->ed_instance_limit) {
                                          mpi_send_support_event(p_device, emc::emc_enable_tag);
                                      }
                                      p_device->ed_instance_count--;
                                  }
                                  // fold the stream count, if and for as long as possible
                                  while(l_stream_index == m_stream_count - 1) {
                                      --l_stream_index;
                                      --m_stream_count;
                                      if(m_search_index >= m_stream_count) {
                                          m_search_index = m_stream_count;
                                      }
                                      if(m_stream_count == 0) {
                                          break;
                                      }
                                      if(m_stream_list[l_stream_index].es_type != edt_none) {
                                          break;
                                      }
                                  }
                                  l_rc = emc::err_okay;
                              } else
                                  l_rc = emc::err_fail;
                          } else
                              l_rc = emc::err_okay;
                      } else
                          l_rc = emc::err_no_request;
                  } else
                      l_rc = emc::err_no_request;
              } else
                  l_rc = emc_error(emc::err_bad_request, "Invalid channel ID");
          } else
              l_rc = emc::err_no_request;
      } else
      if(argv[0].has_text("sync")) {
          if(argc == 1) {
              l_rc = emc::err_fail;
          } else
              l_rc = emc::err_no_request;
      } else
          l_rc = emc::controller::emc_std_process_request(argc, argv);
      return l_rc;
}

int   mapper::emc_std_process_response(int argc, const sys::argv& argv) noexcept
{
      return emc::controller::emc_std_process_response(argc, argv);
}

void  mapper::emc_std_sync(float) noexcept
{
}

void  mapper::describe() noexcept
{
      mpi_send_support_response();
}

/*namespace emc*/ }
