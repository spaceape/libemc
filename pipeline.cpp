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
#include "pipeline.h"
#include "reactor.h"
#include "gateway.h"
#include "error.h"

namespace emc {

/* rawstage
*/
      rawstage::rawstage() noexcept:
      p_stage_prev(nullptr),
      p_stage_next(nullptr),
      p_owner(nullptr)
{
}

      rawstage::~rawstage() noexcept
{
      if(p_owner != nullptr) {
          p_owner->detach(this);
      }
}

void  rawstage::emc_raw_attach(reactor*) noexcept
{
}

bool  rawstage::emc_raw_resume(reactor*) noexcept
{
      return true;
}

void  rawstage::emc_raw_join() noexcept
{
}

void  rawstage::emc_raw_recv(std::uint8_t* data, int size) noexcept
{
      if(p_stage_next != nullptr) {
          p_stage_next->emc_raw_recv(data, size);
      }
}

int   rawstage::emc_raw_feed(std::uint8_t* data, int size) noexcept
{
      if(p_stage_next != nullptr) {
          return p_stage_next->emc_raw_feed(data, size);
      }
      return err_okay;
}

int   rawstage::emc_raw_send(std::uint8_t* data, int size) noexcept
{
      if(p_stage_prev != nullptr) {
          return p_stage_prev->emc_raw_send(data, size);
      } else
      if(p_owner) {
          return p_owner->emc_raw_send(data, size);
      } else
          return err_fail;
}

void  rawstage::emc_raw_suspend(reactor*) noexcept
{
}

int   rawstage::emc_raw_event(int id, void* data) noexcept
{
      if(p_owner) {
          return p_owner->emc_raw_event(id, data);
      }
      return err_okay;
}

void  rawstage::emc_raw_drop() noexcept
{
}

void  rawstage::emc_raw_detach(reactor*) noexcept
{
}

void  rawstage::emc_raw_sync(float) noexcept
{
}

auto  rawstage::get_type() const noexcept -> const char*
{
      return nullptr;
}

/* emcstage
*/
      emcstage::emcstage() noexcept:
      p_stage_prev(nullptr),
      p_stage_next(nullptr),
      p_owner(nullptr)
{
}

      emcstage::~emcstage() noexcept
{
      if(p_owner != nullptr) {
          p_owner->detach(this);
      }
}

void  emcstage::emc_std_attach(gateway*) noexcept
{
}

bool  emcstage::emc_std_resume(gateway*) noexcept
{
      return true;
}

void  emcstage::emc_std_join() noexcept
{
}

void  emcstage::emc_std_connect(const char*, const char*, int) noexcept
{
}

void  emcstage::emc_std_process_message(const char* message, int length) noexcept
{
}

void  emcstage::emc_std_process_error(const char* message, int length) noexcept
{
}

int   emcstage::emc_std_process_request(int argc, const sys::argv& argv) noexcept
{
      if(p_stage_next != nullptr) {
          return p_stage_next->emc_std_process_request(argc, argv);
      }
      return err_no_request;
}

int   emcstage::emc_std_process_response(int argc, const sys::argv& argv) noexcept
{
      if(p_stage_next != nullptr) {
          return p_stage_next->emc_std_process_response(argc, argv);
      }
      return err_no_response;
}

void  emcstage::emc_std_process_comment(const char* message, int length) noexcept
{
}

int   emcstage::emc_std_process_packet(int, int, std::uint8_t*) noexcept
{
      return err_okay;
}

int   emcstage::emc_std_return_message(const char* message, int length) noexcept
{
      if(p_stage_prev != nullptr) {
          return p_stage_prev->emc_std_return_message(message, length);
      } else
      if(p_owner != nullptr) {
          return p_owner->emc_std_return_message(message, length);
      } else
          return err_fail;
}

int   emcstage::emc_std_return_packet(int channel, int size, std::uint8_t* data) noexcept
{
      if(p_stage_prev != nullptr) {
          return p_stage_prev->emc_std_return_packet(channel, size, data);
      } else
      if(p_owner != nullptr) {
          return p_owner->emc_std_return_packet(channel, size, data);
      } else
          return err_fail;
}

void  emcstage::emc_std_disconnect() noexcept
{
}

void  emcstage::emc_std_drop() noexcept
{
}

void  emcstage::emc_std_suspend(gateway*) noexcept
{
}

int   emcstage::emc_std_event(int id, void* data) noexcept
{
      if(p_owner != nullptr) {
          return p_owner->emc_std_event(id, data);
      }
      return err_okay;
}

void  emcstage::emc_std_detach(gateway*) noexcept
{
}

void  emcstage::emc_std_sync(float) noexcept
{
}

auto  emcstage::get_cap_name(int index) const -> const char*
{
      return nullptr;
}

/*namespace emc*/ }
