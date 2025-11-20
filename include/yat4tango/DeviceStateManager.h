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

#ifndef _DEVICE_STATE_MANAGER_H_
#define _DEVICE_STATE_MANAGER_H_

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <tango.h>
#include <yat/memory/SharedPtr.h>
#include <yat/threading/Mutex.h>
#include <yat4tango/CommonHeader.h>

namespace yat4tango
{

//- forward decl
class DSMImpl;

// ============================================================================
//! \class DeviceStateManager
//! \brief Basic state management class
//!
//! \code{.cpp}
//! yat4tango::DeviceStateManager s(host_p);
//! typedef enum States { state_ready, state_acquisition } States;
//! ...
//! s.add(state_ready, Tango::ON, "Ready");
//! s.add(state_acquisition, Tango::RUNNING, "Acquisition in progress...", false);
//! ...
//! s.set(state_ready);
//! ...
//! s.set(state_acquisition);
//! ...
//! //- a alarm condition occured
//! s.set_alarm("alarm!");
//! ...
//! //- end of the alarm condition, the state automatically return to the acquisition one
//! s.clear_alarm();
//! ...
//! //- end of acquisition
//! s.set(state_ready);
//! \endcode
// ============================================================================
class YAT4TANGO_DECL DeviceStateManager
{
public:
  //! \brief c-tor
  DeviceStateManager() {}

  //! \brief c-tor
  //! \param host_p device instance
  DeviceStateManager(Tango::DeviceImpl *host_p);
  
  //! \brief set host
  void host(Tango::DeviceImpl *host_p);

  //! \brief return a copy of the device state
  Tango::DevState state() const;

  //! \brief return a copy of the device status
  std::string status() const;

  //! \brief change the device state
  //! \param state new state
  //! \param status new associated status
  //! \param overwrite if true, the status text will be overrided by a alarm or fault status text
  void set(Tango::DevState state, const std::string& status, bool overwrite = true);

  //! \brief set the status text without changing the non-error state
  void status(const std::string& status);

  //! \brief enter the alarm level
  //!
  //! \verbatim
  //! The device state will change to Tango::ALARM and the status text will change to:
  //! | <alarm text>
  //! | ---
  //! | <non-error status>
  //! or
  //! | <alarm text>
  //! if the 'overwrite' parameter was set to true for the non-error status
  //! \endverbatim
  //!
  //! \param alarm the text associated to the alarm level
  void set_alarm(const std::string& alarm);

  //! \brief exit the alarm level
  void clear_alarm();

  //! \brief enter the error level
  //!
  //! \verbatim
  //! The device state will change to Tango::FAULT and the status text will change to (4 possibilities):
  //! 1. The state level was non-error and the overwrite flag was set to true
  //! | <fault text>
  //! 2. The state level was non-error and the overwrite flag was set to false
  //! | <fault text>
  //! | ---
  //! | <non-error text>
  //! 3. The state level was alarm and the overwrite flag was set to true
  //! | <fault text>
  //! | ---
  //! | <alarm text>
  //! 4. The state level was alarm and the overwrite flag was set to false
  //! | <error text>
  //! | ---
  //! | <alarm text>
  //! | ---
  //! | <non-error text>
  //! \endverbatim
  //!
  //! \param status the text associated to the fault level
  void set_fault(const std::string& status);

  //! \brief exit the fault level
  void clear_fault();

  //! \brief define a predefined <non-error state/text> pair
  void add_state(int id, Tango::DevState state, const std::string& status,
                      bool overwrite = true);

  //! \brief set the state/status using a predefined non-error state
  void set(int id);

  //! \brief set Tango::dev_state method polling period (only apply when the
  //!  state is defined to ON)
  void set_dev_state_polling_period_ms(yat::uint32 ms);

private:
  static std::map<std::string, YAT_SHARED_PTR(DSMImpl)> s_impl_rep;
  static yat::Mutex s_rep_mtx;
  YAT_SHARED_PTR(DSMImpl) m_impl_ptr;
};

} // namespace

#endif // _DEVICE_STATE_MANAGER_H_
