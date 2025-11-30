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
#include "reactor.h"
#include "event.h"
#include "error.h"

constexpr std::uint8_t rem_none = 0u;
constexpr std::uint8_t rem_suspend = 128u;
constexpr std::uint8_t rem_all = 255u;
constexpr std::uint8_t rem_any = 255u;

namespace emc {

      reactor::reactor() noexcept:
      p_stage_head(nullptr),
      p_stage_tail(nullptr),
      p_recv_stage(nullptr),
      p_core_stage(nullptr),
      m_enable_events(rem_any),
      m_record_events(rem_none),
      m_resume_bit(false),
      m_join_bit(false),
      m_open_bit(false),
      m_record_enable(false)
{
}

      reactor::~reactor()
{
      std::uint8_t       l_restore_events;
      sys_suspend_events(l_restore_events, rem_all);
      sys_detach_all();
}

void  reactor::sys_attach(stage* stage_ptr) noexcept
{
      stage* p_stage_prev = p_stage_tail;
      stage* p_stage_next = nullptr;
      auto   l_stage_type = stage_ptr->get_type();
      // if stage is one of the known types - insert in the order of their type value
      if((l_stage_type >= stage_type_gate_base) &&
          (l_stage_type <= stage_type_core_last)) {
          while(p_stage_prev != nullptr) {
              if(l_stage_type >= p_stage_prev->get_type()) {
                  break;
              }
              p_stage_next = p_stage_prev;
              p_stage_prev = p_stage_next->p_stage_prev;
          }
      }
      // link the stage into the list
      if(p_stage_prev != nullptr) {
          p_stage_prev->p_stage_next = stage_ptr;
      } else
          p_stage_head = stage_ptr;
      if(p_stage_next != nullptr) {
          p_stage_next->p_stage_prev = stage_ptr;
      } else
          p_stage_tail = stage_ptr;
      stage_ptr->p_stage_prev = p_stage_prev;
      stage_ptr->p_stage_next = p_stage_next;
}

bool  reactor::sys_resume_all() noexcept
{
      stage* i_stage = p_stage_head;
      while(i_stage != nullptr) {
          if(i_stage->emc_raw_resume(this) == false) {
              sys_suspend_all();
              return false;
          }
          i_stage = i_stage->p_stage_next;
      }
      m_resume_bit = true;
      return true;
}

void  reactor::sys_join_all() noexcept
{
}

void  reactor::sys_close(stage* stage_ptr) noexcept
{
      if(m_open_bit) {
          stage_ptr->emc_raw_proto_down();
      }
}

void  reactor::sys_close_all() noexcept
{
      if(m_open_bit) {
          stage* i_stage = p_stage_tail;
          while(i_stage != nullptr) {
              sys_close(i_stage);
              i_stage = i_stage->p_stage_prev;
          }
          m_open_bit = false;
      }
}

void  reactor::sys_drop(stage* stage_ptr) noexcept
{
      if(m_join_bit) {
          sys_close(stage_ptr);
          stage_ptr->emc_raw_drop();
      }
}

void  reactor::sys_drop_all() noexcept
{
      if(m_join_bit) {
          stage* i_stage = p_stage_tail;
          while(i_stage != nullptr) {
              sys_drop(i_stage);
              i_stage = i_stage->p_stage_prev;
          }
          m_open_bit = false;
          m_join_bit = false;
      }
}

void  reactor::sys_suspend(stage* stage_ptr) noexcept
{
      if(m_resume_bit) {
          sys_drop(stage_ptr);
          stage_ptr->emc_raw_suspend(this);
      }
}

void  reactor::sys_suspend_all() noexcept
{
      if(m_resume_bit) {
          stage* i_stage = p_stage_tail;
          while(i_stage != nullptr) {
              sys_suspend(i_stage);
              i_stage = i_stage->p_stage_prev;
          }
          m_open_bit = false;
          m_join_bit = false;
          m_resume_bit = false;
      }
}

void  reactor::sys_suspend_all(stage* exclude_ptr) noexcept
{
      if(m_resume_bit) {
          stage* i_stage = p_stage_tail;
          while(i_stage != nullptr) {
              if(i_stage != exclude_ptr) {
                  sys_suspend(i_stage);
              }
              i_stage = i_stage->p_stage_prev;
          }
          m_open_bit = false;
          m_join_bit = false;
          m_resume_bit = false;
      }
}

void  reactor::sys_detach(stage* stage_ptr) noexcept
{
      std::uint8_t l_restore_events;
      sys_suspend_events(l_restore_events, rem_suspend);
      sys_record_events();
      stage_ptr->emc_raw_detach(this);
      if(p_core_stage == stage_ptr) {
          p_core_stage = nullptr;
      }
      if(p_recv_stage == stage_ptr) {
          if(((m_record_events & rem_suspend) == false) &&
              ((l_restore_events & rem_suspend) == false)) {
              sys_suspend_all(stage_ptr);
          }
          p_recv_stage = nullptr;
      }
      stage_ptr->p_owner = nullptr;
      if(stage_ptr->p_stage_prev != nullptr) {
          stage_ptr->p_stage_prev->p_stage_next = stage_ptr->p_stage_next;
      } else
          p_stage_head = stage_ptr->p_stage_next;
      if(stage_ptr->p_stage_next != nullptr) {
          stage_ptr->p_stage_next->p_stage_prev = stage_ptr->p_stage_prev;
      } else
          p_stage_tail = stage_ptr->p_stage_prev;
      stage_ptr->p_stage_prev = nullptr;
      stage_ptr->p_stage_next = nullptr;
      sys_delete_events();
      sys_restore_events(l_restore_events);
}

void  reactor::sys_detach_all() noexcept
{
      while(p_stage_tail != nullptr) {
          sys_detach(p_stage_tail);
      }
      m_open_bit = false;
      m_join_bit = false;
      m_resume_bit = false;
}

void  reactor::sys_sync_all(float dt) noexcept
{
      stage* i_stage = p_stage_head;
      while(i_stage != nullptr) {
          i_stage->sync(dt);
          i_stage = i_stage->p_stage_next;
      }
}

void  reactor::sys_suspend_events(std::uint8_t& restore_bits, std::uint8_t disable_bits) noexcept
{
      restore_bits     = m_enable_events;
      m_enable_events &= (~disable_bits);
}

void  reactor::sys_restore_events(std::uint8_t& restore_bits) noexcept
{
      m_enable_events = restore_bits;
}

void  reactor::sys_record_events() noexcept
{
      m_record_events = 0u;
      m_record_enable = true;
}

void  reactor::sys_delete_events() noexcept
{
      m_record_events = 0u;
      m_record_enable = false;
}

bool  reactor::emc_raw_resume() noexcept
{
      return true;
}

int   reactor::emc_raw_recv(int bus, std::uint8_t* data, std::size_t size) noexcept
{
      if(p_stage_head != nullptr) {
          return p_stage_head->emc_raw_recv(bus, data, size);
      } else
          return err_no_response;
}

bool  reactor::emc_raw_suspend() noexcept
{
      return true;
}

void  reactor::emc_raw_sync(float) noexcept
{
}

int   reactor::emc_raw_event(event, const event_info_t&) noexcept
{
      return err_not_required;
}

bool  reactor::pod_resume() noexcept
{
      if(m_resume_bit == false) {
          if(sys_resume_all() == false) {
              return false;
          }
          if(emc_raw_resume() == false) {
              sys_suspend_all();
              return false;
          }
      }
      return m_resume_bit == true;
}

bool  reactor::pod_attach_stage(stage* stage_ptr) noexcept
{
      if(stage_ptr != nullptr) {
          if(stage_ptr->p_owner == nullptr) {
              std::uint8_t l_restore_events;
              if(stage_ptr->has_type(stage_type_gate_base, stage_type_gate_last)) {
                  if(p_recv_stage != nullptr) {
                      return false;
                  }
                  p_recv_stage = stage_ptr;
              } else
              if(stage_ptr->has_type(stage_type_core_base, stage_type_core_base)) {
                  if(p_core_stage != nullptr) {
                      return false;
                  }
                  p_core_stage = stage_ptr;
              }
              sys_attach(stage_ptr);
              sys_suspend_events(l_restore_events, rem_suspend);
              stage_ptr->p_owner = this;
              stage_ptr->emc_raw_attach(this);
              if(m_resume_bit) {
                  if(stage_ptr->emc_raw_resume(this)) {
                      if(m_join_bit) {
                          stage_ptr->emc_raw_join();
                      }
                  } else
                      sys_suspend_all(stage_ptr);
              }
              sys_restore_events(l_restore_events);
              return true;
          } else
          if(stage_ptr->p_owner == this) {
              return true;
          }
      }
      return false;
}

bool  reactor::pod_detach_stage(stage* stage_ptr) noexcept
{
      if(stage_ptr != nullptr) {
          if(stage_ptr->p_owner == this) {
              sys_detach(stage_ptr);
              return true;
          } else
          if(stage_ptr->p_owner == nullptr) {
              return true;
          }
      }
      return false;
}

bool  reactor::pod_suspend(bool send_suspend_event) noexcept
{
      if(m_resume_bit == true) {
          if((send_suspend_event == true) ||
              (emc_raw_suspend() == false)) {
              return false;
          }
          sys_suspend_all();
      }
      return m_resume_bit == false;
}

void  reactor::feed(int) noexcept
{
}

void  reactor::hup(int) noexcept
{
}

int   reactor::post(event id, const event_info_t& info) noexcept
{
      int l_result = emc_raw_event(id, info);
      if(l_result == err_not_required) {
          switch(id) {
            case event::drop:
            case event::hup:
            case event::abort:
            case event::terminated:
            case event::soft_fault:
            case event::hard_fault:
                if(m_record_enable) {
                    m_record_events |= rem_suspend;
                }
                if(m_enable_events & rem_suspend) {
                    if(pod_suspend()) {
                        l_result = err_okay;
                    } else
                        l_result = err_fail;
                } else
                    l_result = err_okay;
                break;
            default:
                break;
          }
      }
      return l_result;
}

void  reactor::sync(float dt) noexcept
{
      sys_sync_all(dt);
      emc_raw_sync(dt);
}

/*namespace emc*/ }
