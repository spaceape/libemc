#ifndef emc_error_h
#define emc_error_h
/** 
    Copyright (c) 2022, wicked systems
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
#include "emc.h"
#include <error.h>

namespace emc {

/* err_okay
   no error, all went smooth
*/
constexpr int  err_okay = ::err_okay;
constexpr char msg_ready[] = "READY";

/* err_parse
   request could not be parsed 
*/
constexpr int  err_parse = -127;
constexpr char msg_parse[] = "INVALID REQUEST";

/* err_bad_request
   request is generally erroneous
*/
constexpr int  err_bad_request = -2;
constexpr char msg_bad_request[] = "BAD REQUEST";


/* err_no_request
   request could not be handled by any attached modules
*/
constexpr int  err_no_request = -1;
constexpr char msg_no_request[] = "COMMAND NOT FOUND";

/* err_no_response
   response could not be handled by the super-session
*/
constexpr int  err_no_response = -1;
constexpr char msg_no_response[] = "COMMAND NOT FOUND";

/* err_fail
*/
constexpr int  err_fail = -128;
constexpr char msg_fail[] = "INTERNAL ERROR";

/*namespace emc*/ }
#endif
