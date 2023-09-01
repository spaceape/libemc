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
#include "controller.h"

namespace emc {

      controller::controller(role role) noexcept:
      p_stage_head(nullptr),
      p_stage_tail(nullptr),
      m_stage_count(0),
      m_role(role)
{
}

      controller::~controller()
{
      rawstage* i_stage = p_stage_tail;
      while(i_stage != nullptr) {
          i_stage->emc_raw_detach(this);
          i_stage = i_stage->p_stage_prev;
      }
}

void  controller::ems_stage_attach(int, rawstage*) noexcept
{
}

void  controller::emc_join() noexcept
{
      if(p_stage_head != nullptr) {
          p_stage_head->emc_raw_join();
      }
}

void  controller::emc_feed(std::uint8_t* data, int size) noexcept
{
      if(p_stage_head != nullptr) {
          p_stage_head->emc_raw_feed(data, size);
      }
}

int   controller::emc_recv(std::uint8_t* data, int size) noexcept
{
      return err_okay;
}

void  controller::emc_drop() noexcept
{
      if(p_stage_tail != nullptr) {
          p_stage_tail->emc_raw_drop();
      }
}

void  controller::ems_stage_detach(int, rawstage*) noexcept
{
}

bool  controller::attach(rawstage* stage) noexcept
{
      if(stage != nullptr) {
          if(stage->p_owner == nullptr) {
              // link the new stage into the controller list
              stage->p_owner = this;
              stage->p_stage_prev = p_stage_tail;
              stage->p_stage_next = nullptr;
              if(p_stage_tail != nullptr) {
                  p_stage_tail->p_stage_next = stage;
              } else
                  p_stage_head = stage;
              p_stage_tail = stage;
              // bring the stage up
              stage->emc_raw_attach(this);
              ++m_stage_count;
              ems_stage_attach(m_stage_count, stage);
          }
          return stage->p_owner == this;
      }
      return false;
}

bool  controller::detach(rawstage* stage) noexcept
{
      if(stage != nullptr) {
          if(stage->p_owner == this) {
              stage->emc_raw_detach(this);
              if(stage->p_stage_prev != nullptr) {
                  stage->p_stage_prev->p_stage_next = stage->p_stage_next;
              } else
                  p_stage_head = stage->p_stage_next;
              if(stage->p_stage_next != nullptr) {
                  stage->p_stage_next->p_stage_prev = stage->p_stage_prev;
              } else
                  p_stage_tail = stage->p_stage_prev;
              stage->p_stage_next = nullptr;
              stage->p_stage_prev = nullptr;
              stage->p_owner = nullptr;
              --m_stage_count;
              ems_stage_detach(m_stage_count, stage);
          }
          return stage->p_owner == nullptr;
      }
      return true;
}

bool  controller::has_role(role value) const noexcept
{
      return m_role == value;
}

auto  controller::get_role() const noexcept -> controller::role
{
      return m_role;
}

auto  controller::get_system_name() noexcept -> const char*
{
      return nullptr;
}

auto  controller::get_system_type() noexcept -> const char*
{
      return nullptr;
}

void  controller::sync(float dt) noexcept
{
      rawstage* i_stage = p_stage_head;
      while(i_stage != nullptr) {
          i_stage->sync(dt);
          i_stage = i_stage->p_stage_next;
      }
}

/*namespace emc*/ }
