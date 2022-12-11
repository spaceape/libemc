#ifndef emc_protocol_h
#define emc_protocol_h
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

constexpr char emc_tag_request = '?';
constexpr char emc_tag_response = ']';
constexpr char emc_tag_sync = '@';
constexpr char emc_tag_help = '!';

constexpr char emc_request_info = 'i';

constexpr char emc_response_info = 'i';
constexpr char emc_response_support = 's';
constexpr char emc_response_pong = 'g';
constexpr char emc_response_okay = '0';
constexpr char emc_response_error = 'e';
constexpr char emc_response_bye = 'z';
constexpr char emc_packet_header_size = 4;  // size of a packet header: 1 byte denoting the channel + 3 for size

constexpr char emc_protocol_name[] = "emc";
constexpr char emc_protocol_version[] = "1.0";

constexpr int  emc_type_size = 8;
constexpr int  emc_name_size = 24;
constexpr int  emc_info_size = 8;

constexpr char emc_machine_name_none[] = "(anonymous)";
constexpr char emc_machine_type_generic[] = "generic";
constexpr char emc_order_le[] = "le";
constexpr char emc_order_be[] = "be";

constexpr char ctl_service_gcode[] = "gcode";
constexpr char ctl_service_telemetry[] = "mtm";

constexpr char emc_generic_ident_chs[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";
constexpr char emc_proto_ident_chs[] = "0123456789abcdefghijklmnopqrstuvwxyz-";
constexpr char emc_machine_ident_chs[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";
constexpr char emc_decimal_chs[] = "0123456789";

/*namespace emc*/ }
#endif
