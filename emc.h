#ifndef emc_h
#define emc_h
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
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
    EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/
#include <global.h>
#include <sys/ios.h>

namespace emc {

/* type_flags
*/
constexpr unsigned int stage_type_none = 0u;
constexpr unsigned int stage_type_gate_base = 0x00000001;
constexpr unsigned int stage_type_gate_last = 0x0000001f;
constexpr unsigned int stage_type_auth_base = 0x00000020;
constexpr unsigned int stage_type_auth_last = 0x0000004f;
constexpr unsigned int stage_type_core_base = 0x00000050;
constexpr unsigned int stage_type_core_last = 0x0000007f;
constexpr unsigned int stage_type_generic = 0x000000ff;
constexpr unsigned int stage_type_bits = 0x000000ff;

/* ring_flags
*/
constexpr unsigned int ring_unknown = 0u;
constexpr unsigned int ring_network = 0u * 0x00000100;
constexpr unsigned int ring_machine = 1u * 0x00000100;
constexpr unsigned int ring_session = 2u * 0x00000100;
constexpr unsigned int ring_process = 3u * 0x00000100;

constexpr unsigned int ring_bits = 0x00000f00;

/* packet_head_size
*/
constexpr int packet_head_size = 4;
constexpr int packet_size_multiplier = 8;
constexpr int packet_size_max = packet_size_multiplier * (1 << (packet_head_size - 1) << 2);

/* chid_*
   channel IDs
*/
constexpr int chid_none = 0;
constexpr int chid_min = 1;
constexpr int chid_max = 127;

/* objects
*/
class reactor;
class stage;
class monitor;
class gateway;
class controller;

/*namespace emc*/ }
#endif
