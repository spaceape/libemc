#ifndef emc_event_h
#define emc_event_h
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

namespace emc {

/* ev_join
   signal the reactor that the endpoint connection is up
*/
static constexpr int  ev_join = 1;

/* ev_drop
   signal the reactor that the endpoint connection is down
*/
static constexpr int  ev_drop = 2;

/* ev_hup
   signal the reactor that the endpoint has hung up
*/
static constexpr int  ev_hup = 3;

/* ev_progess
*/
static constexpr int  ev_progress = 13;

/* ev_soft_fault
   generic fault in one of the stages
*/
static constexpr int  ev_soft_fault = 14;

/* ev_hard_fault
   generic fault in one of the stages; not recoverable, should end up with a suspend
*/
static constexpr int  ev_hard_fault = 15;

/* ev_user_base
*/
static constexpr int  ev_user_base = 16;

/* ev_user_last
*/
static constexpr int  ev_user_last = 255;

/*namespace emc*/ }
#endif
