//----------------------------------------------------------------------------
// Copyright (c) 2004-2025 The Tango Community
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Lesser Public License v3
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/lgpl.html
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// YAT4Tango LIBRARY
//----------------------------------------------------------------------------
//
// Copyright (C) 2006-2025 The Tango Community
//
// The YAT4Tango library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 2 of the License, or (at
// your option) any later version.
//
// The YAT4Tango library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// See COPYING file for license details
//
// Contact:
//      Stephane Poirier
//      Synchrotron SOLEIL
//------------------------------------------------------------------------------
/*!
 * \authors See AUTHORS file
 */

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <yat/threading/Pulser.h>
#include <yat/utils/Float.h>
#include <yat4tango/FormatHelper.h>
#include <yat4tango/LogHelper.h>
#include <yat4tango/DeviceStateManager.h>

namespace yat4tango
{

// ============================================================================
// Implementation class
// ============================================================================
class DSMImpl : yat4tango::TangoLogAdapter
{
private:
  struct Predefined
  {
    Predefined()
    {
      state = Tango::UNKNOWN;
      overwrite = true;
    }

    Predefined(Tango::DevState _state, const std::string& _status, bool _overwrite)
    : state(_state), status(_status), overwrite(_overwrite) {}

    Tango::DevState state;
    std::string status;
    bool overwrite;
  };


public:
  //---------------------------------------------------------------------------
  DSMImpl(Tango::DeviceImpl *host_p): yat4tango::TangoLogAdapter(host_p),
                                      m_host_p(host_p),
                                      m_state(Tango::UNKNOWN),
                                      m_actual_state(Tango::UNKNOWN),
                                      m_polling_ms(1000)
  {}

  //---------------------------------------------------------------------------
  Tango::DevState state() const
  {
    yat::AutoMutex<> _lock(m_mtx);
    return m_actual_state;
  }

  //---------------------------------------------------------------------------
  std::string status() const
  {
    yat::AutoMutex<> _lock(m_mtx);
    return m_actual_status;
  }

  //---------------------------------------------------------------------------
  void set(Tango::DevState state, const std::string& status, bool overwrite = true)
  {
    if( Tango::ALARM == state )
    {
      set_alarm(status);
    }
    else if( Tango::FAULT == state )
    {
      set_fault(status);
    }
    else if( !status.empty() )
    {
      yat::AutoMutex<> _lock(m_mtx);
      m_state = state;
      m_status = status;
      m_overwrite = overwrite;
      priv_state_status();
    }
  }

  //---------------------------------------------------------------------------
  void status(const std::string& status)
  {
    yat::AutoMutex<> _lock(m_mtx);
    m_status = status;
  }

  //---------------------------------------------------------------------------
  void set_alarm(const std::string& alarm)
  {
    if( !alarm.empty() )
    {
      yat::AutoMutex<> _lock(m_mtx);
      m_alarm_status = alarm;
      priv_state_status();
    }
  }

  //---------------------------------------------------------------------------
  void clear_alarm()
  {
    yat::AutoMutex<> _lock(m_mtx);
    if( !m_alarm_status.empty() )
    {
      m_alarm_status.clear();
      priv_state_status();
    }
  }

  //---------------------------------------------------------------------------
  void set_fault(const std::string& status)
  {
    if( !status.empty() )
    {
      yat::AutoMutex<> _lock(m_mtx);
      m_fault_status = status;
      priv_state_status();
    }
  }

  //---------------------------------------------------------------------------
  void clear_fault()
  {
    yat::AutoMutex<> _lock(m_mtx);
    if( !m_fault_status.empty() )
    {
      m_fault_status.clear();
      priv_state_status();
    }
  }

  //---------------------------------------------------------------------------
  void add_state(int id, Tango::DevState state, const std::string& status,
                      bool overwrite = true)
  {
    m_predefined[id] = Predefined(state, status, overwrite);
  }

  //---------------------------------------------------------------------------
  void set(int id)
  {
    if( m_predefined.find(id) != m_predefined.end() )
    {
      if( m_state != m_predefined[id].state )
      {
        set(m_predefined[id].state, m_predefined[id].status, m_predefined[id].overwrite);
      }
    }
  }

  //---------------------------------------------------------------------------
  void set_dev_state_polling_period_ms(yat::uint32 ms)
  {
    yat::AutoMutex<> _lock(m_mtx);
    m_polling_ms = ms;
  }

private:
  mutable yat::Mutex m_mtx;
  Tango::DeviceImpl* m_host_p;
  Tango::DevState    m_state;
  Tango::DevState    m_actual_state;
  std::string        m_status;
  std::string        m_actual_status;
  std::string        m_fault_status;
  std::string        m_alarm_status;
  bool               m_overwrite;
  std::map<int, Predefined> m_predefined;
  YAT_UNIQUE_PTR(yat::Pulser) m_pulser_ptr;
  yat::uint32        m_polling_ms;
  
  void pulser_cb(yat::Thread::IOArg)
  {
    yat::AutoMutex<> _lock(m_mtx);
    priv_state_status();
    if( yat::Float64(m_polling_ms) != m_pulser_ptr->get_period() )
      m_pulser_ptr->set_period(m_polling_ms);
  }

  // Compute the actual state/status values
  void priv_state_status()
  {
    Tango::DevState prev_state = m_actual_state;

    m_actual_status.clear();

    if( !m_fault_status.empty() )
      m_actual_status = m_fault_status;

    if( !m_alarm_status.empty() )
    {
      if( !m_actual_status.empty() )
        m_actual_status.append("\n---\n");

      m_actual_status.append(m_alarm_status);
    }

    if( m_actual_status.empty() )
      m_actual_status = m_status;
    else if( !m_overwrite )
      m_actual_status.append("\n---\n").append(m_status);

    m_actual_state = m_state;
    if( !m_fault_status.empty() )
      m_actual_state = Tango::FAULT;
    if( !m_alarm_status.empty() )
      m_actual_state = Tango::ALARM;

    if( Tango::ON == m_actual_state )
    {
      //- Tango manage the ON state differently than the other states:
      //-   In this case and if a attribute quality fall to invalid due to
      //-   a value outside the defined limits it automatically switch the device
      //-   to ALARM state. In order to detect this, we have to periodically
      //-   check the actual state, but it comes at a cost: all attributes with
      //-   min/max limits defined will be read...
      if( prev_state != m_actual_state )
      {
        if( !m_pulser_ptr )
        {
          yat::Pulser::Config pcfg;
          pcfg.period_in_msecs = m_polling_ms;
          pcfg.callback = yat::PulserCallback::instanciate(*this,
                                                           &DSMImpl::pulser_cb);
          pcfg.user_data = this;
          m_pulser_ptr.reset(new yat::Pulser(pcfg));
        }
        m_pulser_ptr->start();
      }
    }
    else if( m_pulser_ptr && m_pulser_ptr->is_running() )
    {
      m_pulser_ptr->stop();
    }

    //- Tango management
    m_host_p->set_state(m_actual_state);
    m_host_p->set_status(m_actual_status);
    //- let Tango to set the state to ALARM if needed
    m_host_p->Tango::DeviceImpl::dev_state();
    m_actual_state = m_host_p->get_state();
    //- let Tango to amend the status if needed
    m_actual_status = m_host_p->Tango::DeviceImpl::dev_status();

    if( prev_state != m_actual_state)
    {
      Y4T_LOG_INFO("Device state changed to {}.", m_actual_state);
    }
  }
};

// ============================================================================
// DeviceStateManager class
// ============================================================================

//- static members
std::map<std::string, YAT_SHARED_PTR(DSMImpl)> DeviceStateManager::s_impl_rep;
yat::Mutex DeviceStateManager::s_rep_mtx;

//-----------------------------------------------------------------------------
DeviceStateManager::DeviceStateManager(Tango::DeviceImpl *host_p)
{
  host(host_p);
}

//-----------------------------------------------------------------------------
void DeviceStateManager::host(Tango::DeviceImpl *host_p)
{
  std::string dev_name = host_p->get_name_lower();
  yat::AutoMutex<> _lock(s_rep_mtx);
  if( s_impl_rep.find(dev_name) == s_impl_rep.end() )
  {
    m_impl_ptr = new DSMImpl(host_p);
    s_impl_rep[dev_name] = m_impl_ptr;
  }
  else
    m_impl_ptr = s_impl_rep[dev_name];
}

//-----------------------------------------------------------------------------
Tango::DevState DeviceStateManager::state() const
{
  if( m_impl_ptr )
    return m_impl_ptr->state();
  return Tango::UNKNOWN;
}

//-----------------------------------------------------------------------------
std::string DeviceStateManager::status() const
{
  if( m_impl_ptr )
    return m_impl_ptr->status();
  return std::string();
}

//-----------------------------------------------------------------------------
void DeviceStateManager::set(Tango::DevState state, const std::string& status,
                               bool overwrite)
{
  if( m_impl_ptr )
    m_impl_ptr->set(state, status, overwrite);
}

//-----------------------------------------------------------------------------
void DeviceStateManager::status(const std::string& status)
{
  if( m_impl_ptr )
    m_impl_ptr->status(status);
}

//-----------------------------------------------------------------------------
void DeviceStateManager::set_alarm(const std::string& status)
{
  if( m_impl_ptr )
    m_impl_ptr->set_alarm(status);
}

//-----------------------------------------------------------------------------
void DeviceStateManager::clear_alarm()
{
  if( m_impl_ptr )
    m_impl_ptr->clear_alarm();
}

//-----------------------------------------------------------------------------
void DeviceStateManager::set_fault(const std::string& status)
{
  if( m_impl_ptr )
    m_impl_ptr->set_fault(status);
}

//-----------------------------------------------------------------------------
void DeviceStateManager::clear_fault()
{
  if( m_impl_ptr )
    m_impl_ptr->clear_fault();
}

//-----------------------------------------------------------------------------
void DeviceStateManager::add_state(int id, Tango::DevState state,
                                   const std::string& status, bool overwrite)
{
  if( m_impl_ptr )
    m_impl_ptr->add_state(id, state, status, overwrite);
}

//-----------------------------------------------------------------------------
void DeviceStateManager::set(int id)
{
  if( m_impl_ptr )
    m_impl_ptr->set(id);
}

//-----------------------------------------------------------------------------
void DeviceStateManager::set_dev_state_polling_period_ms(yat::uint32 ms)
{
  if( m_impl_ptr )
    m_impl_ptr->set_dev_state_polling_period_ms(ms);
}

} // namespace
