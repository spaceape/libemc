#ifndef emc_command_h
#define emc_command_h
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
#include <sys.h>

namespace emc {

class arg;
class command;

/* arg
   argument retrieved by a command line parser
*/
class arg
{
  char* m_text;
  int   m_size;

  public:
          arg() noexcept;
          arg(char*) noexcept;
          arg(char*, int) noexcept;
          arg(const arg&) noexcept;
          arg(arg&&) noexcept;
          ~arg();

          bool   has_key(const char*) const noexcept;
          bool   has_value(const char*) const noexcept;

          bool   has_text() const noexcept;
          bool   has_text(const char*) const noexcept;
          bool   has_text(const char*, int, int) const noexcept;
          char*  get_text() const noexcept;

          int    get_char(int) const noexcept;

          int    get_bin_int() const noexcept;
          int    get_dec_int() const noexcept;
          int    get_hex_int() const noexcept;
          int    get_int() const noexcept;

          bool   has_size(int) const noexcept;
          int    get_size() const noexcept;

          int    operator[](int) const noexcept;
          bool   operator==(const char*) const noexcept;
          bool   operator==(int) const noexcept;
          bool   operator!=(const char*) const noexcept;
          bool   operator!=(int) const noexcept;

  inline         operator char*() const noexcept {
          return get_text();
  }

  inline         operator int() const noexcept {
          return get_int();
  }

          arg&   operator=(const arg&) noexcept;
          arg&   operator=(arg&&) noexcept;
};

/* command
   generic command line parser
*/
class command
{
  /* arg_near_reserve
     how many arg instances to keep within the 'near' array
  */
  static constexpr int   arg_near_reserve = 6;

  /* arg_count_max
    maximum number of arguments to record
  */ 
  static constexpr int   arg_count_max = 16;

  public:
  static constexpr char  arg_split_space[] = " \t\r\n";

  private:
  arg       m_arg_none;
  arg       m_arg_near[arg_near_reserve];   // statically allocated list for args with index [0..arg_near_reserve]
  arg*      m_arg_far;                      // dynamically allocated list for args with index [arg_far_reserve...]
  short int m_arg_count;
  short int m_far_reserve;                  // how many args are available onto the 'far' argument list

  private:
          bool         arg_reserve(int, int) noexcept;
          int          arg_extract_next(char*&, char*&) noexcept;

  public:
          command() noexcept;
          command(char*, int = arg_count_max) noexcept;
          command(char**, int) noexcept;
          command(const command&) noexcept;
          command(command&) noexcept;
          ~command();

          void        assign(const command&) noexcept;
          void        assign(command&&) noexcept;
          int         load(char*, int = arg_count_max) noexcept;
          int         load(char**, int = arg_count_max) noexcept;
          
          const arg&  get_arg(int) const noexcept;
          int         get_index_of(const char*) const noexcept;
          int         get_index_of(arg&) const noexcept;
   
          bool        has_arg(const char*) const noexcept;
          const arg&  get_arg(const char*) const noexcept;
          const arg&  get_arg_relative(const char*, int) const noexcept;

          bool        has_count(int) const noexcept;
          int         get_count() const noexcept;
          void        clear() noexcept;
          void        release() noexcept;
          void        dispose(bool = true) noexcept;
          const arg&  operator[](int) const noexcept;
          command&    operator=(const command&) noexcept;
          command&    operator=(command&&) noexcept;
};

/*namespace sys*/ }
#endif
