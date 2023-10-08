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
#include "reactor.h"
#include "event.h"
#include "error.h"

namespace emc {

      reactor::reactor(role role) noexcept:
      p_stage_head(nullptr),
      p_stage_tail(nullptr),
      m_stage_count(0),
      m_role(role),
      m_resume_bit(false),
      m_connect_bit(false)
{
}

      reactor::~reactor()
{
      rawstage* i_stage = p_stage_tail;
      while(i_stage != nullptr) {
          if(m_resume_bit == true) {
              if(m_connect_bit == true) {
                  i_stage->emc_raw_drop();
              }
              i_stage->emc_raw_suspend(this);
          }
          i_stage->emc_raw_detach(this);
          i_stage = i_stage->p_stage_prev;
      }
}

bool  reactor::ems_resume_at(rawstage* i_stage) noexcept
{
      rawstage* i_stage_prev = i_stage->p_stage_prev;
      while(i_stage != nullptr) {
          if(i_stage->emc_raw_resume(this) == false) {
              ems_suspend_at(i_stage_prev);
              return false;
          }
          i_stage_prev = i_stage;
          i_stage      = i_stage->p_stage_next;  
      }
      return true;
}

void  reactor::ems_suspend_at(rawstage* i_stage) noexcept
{
      while(i_stage != nullptr) {
          i_stage->emc_raw_suspend(this);
          i_stage = i_stage->p_stage_prev;
      }
}

void  reactor::ems_dispatch_join() noexcept
{
      rawstage* i_stage = p_stage_head;
      while(i_stage != nullptr) {
          i_stage->emc_raw_join();
          i_stage = i_stage->p_stage_next;
      }
}

void  reactor::ems_dispatch_drop() noexcept
{
      rawstage* i_stage = p_stage_tail;
      while(i_stage != nullptr) {
          i_stage->emc_raw_drop();
          i_stage = i_stage->p_stage_prev;
      }
}

bool  reactor::emc_raw_resume() noexcept
{ 
      if(m_resume_bit == false) {
          m_resume_bit = ems_resume_at(p_stage_head);
          if(m_resume_bit == true) {
              if(m_connect_bit == true) {
                  ems_dispatch_join();
              }
          }
      }
      return m_resume_bit;
}

void  reactor::emc_raw_join() noexcept
{
      if(m_connect_bit == false) {
          if(m_resume_bit == true) {
              ems_dispatch_join();
          }
          m_connect_bit = true;
      }
}

void  reactor::emc_raw_feed(std::uint8_t* data, int size) noexcept
{
      if(p_stage_head != nullptr) {
          p_stage_head->emc_raw_feed(data, size);
      }
}

int   reactor::emc_raw_recv(std::uint8_t* data, int size) noexcept
{
      return err_okay;
}

void  reactor::emc_raw_drop() noexcept
{
      if(m_connect_bit == true) {
          if(m_resume_bit == true) {
              ems_dispatch_drop();
          }
          m_connect_bit = false;
      }
}

void  reactor::emc_raw_event(int id, void*) noexcept
{
      switch(id) {
          case ev_join:
              emc_raw_join();
              break;
          case ev_drop:
              emc_raw_drop();
              break;
          case ev_hard_fault:
              emc_raw_suspend();
              break;
          default:
              break;
      }
}

void  reactor::emc_raw_suspend() noexcept
{
      if(m_resume_bit == true) {
          if(m_connect_bit == true) {
              ems_dispatch_drop();
              m_connect_bit = false;
          }
          ems_suspend_at(p_stage_tail);
          m_resume_bit = false;
      }
}

void  reactor::ems_sync(float) noexcept
{
}

bool  reactor::has_role(role value) const noexcept
{
      return m_role == value;
}

auto  reactor::get_role() const noexcept -> reactor::role
{
      return m_role;
}

bool  reactor::attach(rawstage* stage) noexcept
{
      if(stage != nullptr) {
          if(stage->p_owner == nullptr) {
              // prepare the stage
              stage->p_owner = this;
              stage->emc_raw_attach(this);
              if(m_resume_bit) {
                  if(stage->emc_raw_resume(this) == false) {
                      emc_raw_suspend();
                      stage->p_owner = nullptr;
                      return false;
                  }
                  if(m_connect_bit) {
                      stage->emc_raw_join();
                  }
              }
              // link the new stage into the reactor list
              stage->p_stage_prev = p_stage_tail;
              stage->p_stage_next = nullptr;
              if(p_stage_tail != nullptr) {
                  p_stage_tail->p_stage_next = stage;
              } else
                  p_stage_head = stage;
              p_stage_tail = stage;
              // bring the stage up
              m_stage_count++;
          }
          return stage->p_owner == this;
      }
      return false;
}

bool  reactor::detach(rawstage* stage) noexcept
{
      if(stage != nullptr) {
          if(stage->p_owner == this) {
              if(m_resume_bit) {
                  if(m_connect_bit) {
                      stage->emc_raw_drop();
                  }
                  stage->emc_raw_suspend(this);
              }
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
              m_stage_count--;
          }
          return stage->p_owner == nullptr;
      }
      return true;
}

bool  reactor::attach(emcstage*) noexcept
{
      return false;
}

bool  reactor::detach(emcstage*) noexcept
{
      return false;
}

auto  reactor::get_system_name() noexcept -> const char*
{
      return nullptr;
}

auto  reactor::get_system_type() noexcept -> const char*
{
      return nullptr;
}

bool  reactor::has_ring_flags(unsigned int ring) const noexcept
{
      return get_ring_flags() >= ring;
}

auto  reactor::get_ring_flags() const noexcept -> unsigned int
{
      return emi_ring_network;
}

bool  reactor::get_valid_state() const noexcept
{
      return p_stage_head != nullptr;
}

bool  reactor::get_resume_state(bool value) const noexcept
{
      return m_resume_bit == value;
}

bool  reactor::get_connect_state(bool value) const noexcept
{
      return m_connect_bit == value;
}

int   reactor::get_stage_count() const noexcept
{
      return m_stage_count;
}

void  reactor::sync(float dt) noexcept
{
      rawstage* i_stage = p_stage_head;
      while(i_stage != nullptr) {
          i_stage->emc_raw_sync(dt);
          i_stage = i_stage->p_stage_next;
      }
      ems_sync(dt);
}

/*namespace emc*/ }
