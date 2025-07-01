/**
    Copyright (c) 2025, wicked systems
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
#include "stage.h"
#include "reactor.h"

namespace emc {

/* stage
*/
      stage::stage(unsigned int flags) noexcept:
      p_stage_prev(nullptr),
      p_stage_next(nullptr),
      p_owner(nullptr),
      m_flags(flags)
{
}

      stage::~stage() noexcept
{
      if(p_owner != nullptr) {
          p_owner->detach(this);
      }
}

auto  stage::emc_get_owner() noexcept -> reactor*
{
      return p_owner;
}

/* emc_raw_attach()
   called when the stage is attached onto a pipeline
*/
void  stage::emc_raw_attach(reactor*) noexcept
{
}

/* emc_raw_resume()
   called when the stage is resumed
*/
bool  stage::emc_raw_resume(reactor*) noexcept
{
      return true;
}

/* emc_raw_join()
   triggered for every stage when the reactor joined the network
*/
void  stage::emc_raw_join() noexcept
{
}

/* emc_raw_proto_up()
   triggered for every stage when the high level protocol negociation succeeds
*/
void  stage::emc_raw_proto_up(const char*, const char*, unsigned int) noexcept
{
}

/* emc_raw_recv()
   called from upstream when a message is received onto the aux (or binary) stream
*/
void  stage::emc_raw_recv(std::uint8_t* data, std::size_t size) noexcept
{
      if(has_owner() &&
          has_kind(emi_kind_gate_base, emi_kind_gate_last)) {
          // input stage, notify the reactor with a pointer to the data buffer
          if(p_owner != nullptr) {
              p_owner->post(ev_recv_aux, event_t::for_packet(data, size));
          }
      }
      if(p_stage_next != nullptr) {
          p_stage_next->emc_raw_recv(data, size);
      }
}

/* emc_raw_feed()
   called from upstream when a message is received onto the std stream
 */
int   stage::emc_raw_feed(std::uint8_t* data, std::size_t size) noexcept
{
      int l_result = err_refuse;
      if(has_owner() &&
          has_kind(emi_kind_gate_base, emi_kind_gate_last)) {
          // input stage, notify the reactor with a pointer to the data buffer
          if(p_owner != nullptr) {
              l_result = p_owner->post(ev_recv_std, event_t::for_packet(data, size));
          }
      }
      if(l_result == err_refuse) {
          if(p_stage_next != nullptr) {
              l_result = p_stage_next->emc_raw_feed(data, size);
          }
      }
      return l_result;
}

/* emc_raw_send()
   called on the return path when a message is to be sent back
*/
int   stage::emc_raw_send(std::uint8_t* data, std::size_t size) noexcept
{
      if(p_stage_prev != nullptr) {
          return p_stage_prev->emc_raw_send(data, size);
      } else
      if(p_owner != nullptr) {
          int  l_result = err_fail;
          // first stage, notify the reactor with a pointer to the data buffer
          if(p_owner != nullptr) {
              l_result = p_owner->post(ev_send, event_t::for_packet(data, size));
              if((l_result == err_okay) ||
                  (l_result == err_refuse)) {
                  l_result = err_okay;
              }
          }
          return l_result;
      }
      return err_okay;
}

/* emc_raw_proto_down()
   triggered for every stage when the high level protocol communication fails or it's suspended
*/
void  stage::emc_raw_proto_down() noexcept
{
}

/* emc_raw_drop()
   triggered for every stage when the reactor dropped from the network
*/
void  stage::emc_raw_drop() noexcept
{
}

/* emc_raw_suspend()
   called when the stage is suspended
*/
void  stage::emc_raw_suspend(reactor*) noexcept
{
}

/* emc_raw_detach()
   called when the stage is removed from the pipeline
*/
void  stage::emc_raw_detach(reactor*) noexcept
{
}

/* emc_raw_sync()
   synchronous update event called periodically
*/
void  stage::emc_raw_sync(float) noexcept
{
}

/* emc_raw_post()
*/
void  stage::emc_raw_post(int id) noexcept
{
      event_t l_event;
      emc_raw_post(id, l_event);
}

/* emc_raw_event()
   custom event called on the return path (i.e. on unexpected stage failures)
*/
int   stage::emc_raw_post(int id, const event_t& event) noexcept
{
      if(p_owner != nullptr) {
          return p_owner->post(id, event);
      }
      return err_fail;
}

/* has_owner()
*/
bool  stage::has_owner() const noexcept
{
      return (p_owner != nullptr);
}

/* has_owner()
*/
bool  stage::has_owner(reactor* reactor) const noexcept
{
      return (p_owner == reactor);
}

/* get_owner()
*/
auto  stage::get_owner() noexcept -> reactor*
{
      return p_owner;
}

/* has_type()
*/
bool  stage::has_type(const char*) const noexcept
{
      return false;
}

/* get_type()
*/
auto  stage::get_type() const noexcept -> const char*
{
      return nullptr;
}

/* has_kind()
*/
bool  stage::has_kind(unsigned int flags) const noexcept
{
      return (m_flags & emi_kind_bits & flags) != 0u;
}

/* has_kind()
*/
bool  stage::has_kind(unsigned int kind_range_min, unsigned int kind_range_max) const noexcept
{
      unsigned int l_kind = (m_flags & emi_kind_bits);
      if((l_kind >= kind_range_min) &&
          (l_kind <= kind_range_max)) {
          return true;
      }
      return false;
}

/* get_layer_flags()
*/
auto  stage::get_kind() const noexcept -> unsigned int
{
      return (m_flags & emi_kind_bits);
}

/* get_layer_name()
*/
auto  stage::get_layer_name(int) const noexcept -> const char*
{
      return nullptr;
}

/* get_layer_flags()
*/
bool  stage::has_ring_flags(unsigned int flags) const noexcept
{
      return (m_flags && emi_ring_bits && flags) != 0u;
}

/* get_layer_flags()
*/
unsigned int stage::get_ring_flags() const noexcept
{
      return (m_flags & emi_ring_bits);
}

/* describe()
*/
void  stage::describe() noexcept
{
}

/* sync()
*/
void  stage::sync(float dt) noexcept
{
      emc_raw_sync(dt);
}

/*namespace emc*/ }
