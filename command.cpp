/** 
    Copyright (c) 2021, wicked systems
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
#include "command.h"
#include <convert.h>
#include <cstring>
#include <sys.h>
#include <log.h>

namespace emc {

/* arg
*/
      arg::arg() noexcept:
      m_text(nullptr),
      m_size(0)
{
}

      arg::arg(char* text) noexcept:
      m_text(text),
      m_size(std::strlen(text))
{
}

      arg::arg(char* text, int size) noexcept:
      m_text(text),
      m_size(size)
{
}

      arg::arg(const arg& copy) noexcept:
      m_text(copy.m_text),
      m_size(copy.m_size)
{
}

      arg::arg(arg&& copy) noexcept:
      m_text(copy.m_text),
      m_size(copy.m_size)
{
}

      arg::~arg()
{
}

int   arg::get_char(int index) const noexcept
{
      if(index < m_size) {
          return m_text[index];
      } else
          return 0;
}

bool  arg::has_key(const char* value) const noexcept
{
      return false;
}

bool  arg::has_value(const char* value) const noexcept
{
      return false;
}

bool  arg::has_text() const noexcept
{
      return m_text;
}

bool  arg::has_text(const char* text) const noexcept
{
      if(text) {
          return std::strncmp(m_text, text, m_size) == 0;
      }
      return false;
}

bool  arg::has_text(const char* text, int offset, int length) const noexcept
{
      if(text) {
          if(offset < m_size) {
              int l_length_max = m_size - offset;
              if(length > l_length_max) {
                  length = l_length_max;
              }
              if(length > 0) {
                  return std::strncmp(m_text, text,  length) == 0;
              }
          }
      }
      return false;
}

char* arg::get_text() const noexcept
{
      return m_text;
}

int   arg::get_bin_int() const noexcept
{
      int l_result = 0;
      convert<char*>::get_bin_value(l_result, m_text, m_size);
      return l_result;
}

int   arg::get_dec_int() const noexcept
{
      int l_result = 0;
      convert<char*>::get_dec_value(l_result, m_text, m_size);
      return l_result;
}

int   arg::get_hex_int() const noexcept
{
      int l_result = 0;
      convert<char*>::get_hex_value(l_result, m_text, m_size);
      return l_result;
}

int   arg::get_int() const noexcept
{
      int l_result = 0;
      convert<char*>::get_int_value(l_result, m_text, m_size, true);
      return l_result;
}

bool  arg::has_size(int size) const noexcept
{
      return m_size == size;
}

int   arg::get_size() const noexcept
{
      return m_size;
}

int   arg::operator[](int index) const noexcept
{
      return get_char(index);
}

bool  arg::operator==(const char* text) const noexcept
{
      return has_text(text);
}

bool  arg::operator!=(const char* text) const noexcept
{
      return has_text(text) == false;
}

arg&  arg::operator=(const arg& rhs) noexcept
{
      m_text = rhs.m_text;
      m_size = rhs.m_size;
      return *this;
}

arg&  arg::operator=(arg&& rhs) noexcept
{
      m_text = rhs.m_text;
      m_size = rhs.m_size;
      return *this;
}

/* command
*/
      command::command() noexcept:
      m_arg_none(),
      m_arg_far(nullptr),
      m_arg_count(0),
      m_far_reserve(0)
{
}

      command::command(char* line, int count_max) noexcept:
      command()
{
      load(line, count_max);
}

      command::command(char** argv, int argc) noexcept:
      command()
{
      load(argv, argc);
}

      command::command(const command& copy) noexcept:
      command()
{
      assign(copy);
}

      command::command(command& copy) noexcept:
      command()
{
      assign(std::move(copy));
}

      command::~command()
{
      dispose(false);
}

bool  command::arg_reserve(int count, int keep) noexcept
{
      int l_far_count = count - arg_near_reserve;   // how many 'far' instances to reserve memory for
      int l_far_move  = keep;                       // how many instances to preserve into the new array
      if(l_far_count > m_far_reserve) {
          int   l_far_reserve = get_round_value(l_far_count, global::cache_small_max);
          arg*  l_far_ptr     = reinterpret_cast<arg*>(malloc(l_far_reserve * sizeof(arg)));
          if(l_far_ptr != nullptr) {
              int i_far_arg = 0;
              // initialise newly allocated arg vector: move existing arg instances to the new vector
              // and destruct the old ones
              if(m_arg_far != nullptr) {
                  while(i_far_arg < l_far_move - arg_near_reserve) {
                      new(l_far_ptr + i_far_arg) arg(std::move(m_arg_far[i_far_arg]));
                      m_arg_far[i_far_arg].~arg();
                      i_far_arg++;
                  }
              }
              // initialise newly allocated arg vector: construct the new instances
              while(i_far_arg < l_far_reserve - arg_near_reserve) {
                  new(l_far_ptr + i_far_arg) arg();
                  i_far_arg++;
              }
              // free the memory of the old array
              free(m_arg_far);
              // store the new array
              m_arg_far     = l_far_ptr;
              m_far_reserve = l_far_reserve;
              return true;
          }
          return false;
      }
      return true;
}

int   command::arg_extract_next(char*& arg_str, char*& arg_iter) noexcept
{
      int  l_end_min = 0;
      int  l_end_max = SPC;

      // handle "" and '' arguments
      if(arg_str[0] == '"') {
          l_end_min = '"';
          l_end_max = '"';
          arg_iter[0] = 0;
          ++arg_iter;
          ++arg_str;
      } else
      if(arg_str[0] == '\'') {
          l_end_min = '\'';
          l_end_max = '\'';
          arg_iter[0] = 0;
          ++arg_iter;
          ++arg_str;
      }
      // parse the argument presumably starting at `arg_iter`: iterate until the first character within the range
      // `l_end_min`..`l_end_max` is encountered.
      while(*arg_iter <= ASCII_MAX) {
          if((*arg_iter >= l_end_min) &&
              (*arg_iter <= l_end_max)) {
              *arg_iter = 0;
              return arg_iter - arg_str;
          }
          arg_iter++;
      }
      return -1;
}

void  command::assign(const command& copy) noexcept
{
      if(this != std::addressof(copy)) {
          clear();
          if(copy.m_arg_count > 0) {
              bool l_reserve_success = arg_reserve(copy.m_arg_count, 0);
              if(l_reserve_success) {
                  for(int 
                        i_arg = 0;
                        (i_arg < copy.m_arg_count) && (i_arg < arg_near_reserve);
                        i_arg++) {
                        m_arg_near[i_arg] = copy.m_arg_near[i_arg];
                  }
                  for(int
                        i_arg = arg_near_reserve;
                        i_arg < copy.m_arg_count;
                        i_arg++) {
                        m_arg_far[i_arg - arg_near_reserve] = copy.m_arg_far[i_arg - arg_near_reserve];
                          
                  }
                  m_arg_count = copy.m_arg_count;
              }
          }
      }
}

void  command::assign(command&& copy) noexcept
{
      if(this != std::addressof(copy)) {
          dispose();
          clear();
          if(copy.m_arg_count > 0) {
              for(int 
                    i_arg = 0;
                    (i_arg < copy.m_arg_count) && (i_arg < arg_near_reserve);
                    i_arg++) {
                    m_arg_near[i_arg] = copy.m_arg_near[i_arg];
              }
              m_arg_far = copy.m_arg_far;
              m_arg_count = copy.m_arg_count;
              copy.release();
          }
      }
      copy.clear();
}

/* load()
   build argument list from string
*/
int   command::load(char* str, int) noexcept
{
      char* l_str_iter  = str;
      int   l_arg_index = 0;
      while(*l_str_iter) {
          if((*l_str_iter >= SPC) &&
              (*l_str_iter <= ASCII_MAX)) {
              char* l_arg_base = l_str_iter;
              int   l_arg_length = arg_extract_next(l_arg_base, l_str_iter);
              if(l_arg_length >= 0) {
                  if(l_arg_index < arg_near_reserve) {
                      m_arg_near[l_arg_index] = arg(l_arg_base, l_arg_length);
                  } else
                  if(l_arg_index < arg_count_max) {
                      int  l_far_index   = l_arg_index - arg_near_reserve;
                      bool l_far_success = arg_reserve(l_arg_index + 1, l_arg_index);
                      if(l_far_success) {
                          m_arg_far[l_far_index] = arg(l_arg_base, l_arg_length);
                      }
                      else {
                          printdbg(
                              "Failed to resize argument list.\n",
                              __FILE__,
                              __LINE__
                          );
                          l_arg_index = 0;
                          break;
                      }
                  }
                  else {
                      printdbg(
                          "Too many argument list entries.\n",
                          __FILE__,
                          __LINE__
                      );
                      l_arg_index = 0;
                      break;
                  }
                  l_arg_index++;
              }
              else {
                  printdbg(
                      "Error while parsing argument %d.\n"
                      " \"  %s \" ",
                      __FILE__,
                      __LINE__,
                      l_arg_index,
                      str
                  );
                  l_arg_index = 0;
                  break;
              }
          }
          l_str_iter++;
      }
      m_arg_count = l_arg_index;
      return m_arg_count;
}

int   command::load(char** argv, int argc) noexcept
{
      m_arg_count = 0;
      if((argc > 0) &&
          (argc < arg_count_max)) {
          bool l_reserve_success = arg_reserve(argc, 0);
          if(l_reserve_success) {
              for(int 
                    i_arg = 0;
                    (i_arg < argc) && (i_arg < arg_near_reserve);
                    i_arg++) {
                    m_arg_near[i_arg] = arg(argv[i_arg]);
              }
              for(int
                    i_arg = arg_near_reserve;
                    i_arg < argc;
                    i_arg++) {
                    m_arg_far[i_arg - arg_near_reserve] = arg(argv[i_arg]);
                      
              }
              m_arg_count = argc;
          }
      }
      return m_arg_count;
 }

const arg& command::get_arg(int index) const noexcept
{
      if(index >= 0) {
          if(index < arg_near_reserve) {
              return m_arg_near[index];
          }
          return m_arg_far[index - arg_near_reserve];
      }
      return m_arg_none;
}

int   command::get_index_of(const char* text) const noexcept
{
      if((text != nullptr) &&
          (text[0] != 0)) {
          for(int i_arg = 0; i_arg < m_arg_count; i_arg++) {
              const arg& r_arg = get_arg(i_arg);
              if(r_arg.has_text(text)) {
                  return i_arg;
              }
              i_arg++;
          }
      }
      return -1;
}

int   command::get_index_of(arg& inst) const noexcept
{
      int i_arg = 0;
      while(i_arg < m_arg_count) {
          const arg& l_arg = get_arg(i_arg);
          if(std::addressof(inst) == std::addressof(l_arg)) {
              return i_arg;
          }
          i_arg++;
      }
      return -1;
}

bool  command::has_arg(const char* name) const noexcept
{
      return get_index_of(name) >= 0;
}

const arg& command::get_arg(const char* name) const noexcept
{
      int  i_arg = get_index_of(name);
      if(i_arg >= 0) {
          return get_arg(i_arg);
      }
      return m_arg_none;
}

const arg& command::get_arg_relative(const char* base, int offset) const noexcept
{
      int  i_arg = get_index_of(base);
      if(i_arg >= 0) {
          int i_offset = i_arg + offset;
          if(i_offset < m_arg_count) {
              return get_arg(i_offset);
          }
      }
      return m_arg_none;
}

bool  command::has_count(int count) const noexcept
{
      return get_count() == count;
}

int   command::get_count() const noexcept
{
      return m_arg_count;
}

void  command::clear() noexcept
{
      m_arg_count = 0;
}

void  command::release() noexcept
{
      m_arg_far = nullptr;
      m_far_reserve = 0;
      if(m_arg_count > arg_near_reserve) {
          m_arg_count = arg_near_reserve;
      }
}

void  command::dispose(bool reset) noexcept
{
      if(m_arg_far != nullptr) {
          for(int
                i_arg = arg_near_reserve;
                i_arg < m_arg_count;
                i_arg++) {
                m_arg_far[i_arg - arg_near_reserve].~arg();
          }
          free(m_arg_far);
          if(reset) {
              release();
          }
      }
}

const arg&  command::operator[](int index) const noexcept
{
      return get_arg(index);
}

command&  command::operator=(const command& rhs) noexcept
{
      assign(rhs);
      return *this;
}

command&  command::operator=(command&& rhs) noexcept
{
      assign(std::move(rhs));
      return *this;
}

/*namespace emc*/ }
