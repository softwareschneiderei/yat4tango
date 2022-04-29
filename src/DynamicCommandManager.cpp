//----------------------------------------------------------------------------
// Copyright (c) 2004-2016 The Tango Community
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Lesser Public License v3
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/lgpl.html
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// YAT4Tango LIBRARY
//----------------------------------------------------------------------------
//
// Copyright (c) 2004-2016 The Tango Community
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
//      Nicolas Leclercq
//      Synchrotron SOLEIL
//------------------------------------------------------------------------------
/*!
 * \authors See AUTHORS file
 */

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include "yat/utils/StringTokenizer.h"
#include <yat4tango/DynamicCommandManager.h>

namespace yat4tango
{

// ============================================================================
// DynamicCommandManager::ctor
// ============================================================================
DynamicCommandManager::DynamicCommandManager(Tango::DeviceImpl * _dev)
  : Tango::LogAdapter(_dev), num_pogo_cmd_(0), dev_(_dev)
{
  YAT4TANGO_TRACE("DynamicCommandManager::DynamicCommandManager");
}

// ============================================================================
// DynamicCommandManager::dtor
// ============================================================================
DynamicCommandManager::~DynamicCommandManager()
{
  YAT4TANGO_TRACE("DynamicCommandManager::~DynamicCommandManager");

  this->remove_commands();
}

// ============================================================================
// DynamicCommandManager::set_host_device
// ============================================================================
void DynamicCommandManager::set_host_device (Tango::DeviceImpl * h)
{
  if ( this->dev_ && h != this->dev_ )
  {
    THROW_DEVFAILED( "CONFIGURATION_ERROR",
                     "can't change the Tango device associated with a DynamicCommandManager [already linked to a device]",
                     "DynamicCommandManager::add_command");
  }

  if ( ! h )
  {
    THROW_DEVFAILED( "INVALID_ARG",
                     "unvalid Tango::DeviceImpl specified [unexcpected null pointer]",
                     "DynamicCommandManager::add_command");
  }

  this->dev_ = h;
  this->num_pogo_cmd_ = h->get_device_class()->get_command_list().size();
}

// ============================================================================
// DynamicCommandManager::add_commands
// ============================================================================
void DynamicCommandManager::add_commands (const std::vector<DynamicCommandInfo>& ci)
{
  YAT4TANGO_TRACE("DynamicCommandManager::add_commands");

  for (size_t i = 0; i < ci.size(); i++)
    this->add_command(ci[i]);
}

// ============================================================================
// DynamicCommandManager::add_command
// ============================================================================
void DynamicCommandManager::add_command (const yat4tango::DynamicCommandInfo& dci)
{
  YAT4TANGO_TRACE("DynamicCommandManager::add_command[const yat4tango::DynamicCommandInfo&]");

  if ( ! this->dev_ )
  {
    THROW_DEVFAILED( "INITIALIZATION_ERROR",
                     "the DynamicCommandManager is not properly initialized [no associated device]",
                     "DynamicCommandManager::add_command");
  }

  //- check command does not already exist
  DynamicCommandIterator it = this->rep_.find(dci.tci.cmd_name);
  if (it != this->rep_.end())
  {
    std::ostringstream oss;
    oss << "couldn't add dynamic command <"
        << dci.tci.cmd_name
        << "> - another dynamic command already exists with same name";
    THROW_DEVFAILED("OPERATION_NOT_ALLOWED",
                    oss.str().c_str(),
                    "DynamicCommandManager::add_command");
  }

  //- instanciate the dynamic command (might throw an exception)
  DynamicCommand * dc = DynamicCommandFactory::instance().create_command(dci);

  //- add it to both the device and the repository
  try
  {
    this->add_command(dc);
  }
  catch (Tango::DevFailed&)
  {
    delete dc;
    throw;
  }
  catch (...)
  {
    delete dc;
    THROW_DEVFAILED( "UNKNOWN_ERROR",
                     "unknown exception caught while trying to add the dynamic command to the device interface",
                     "DynamicCommandManager::add_command");
  }
}

// ============================================================================
// DynamicCommandManager::add_command
// ============================================================================
void DynamicCommandManager::add_command (yat4tango::DynamicCommand * dc)
{
  YAT4TANGO_TRACE("DynamicCommandManager::add_command[DynamicCommand*]");

  if ( ! this->dev_ )
  {
    THROW_DEVFAILED( "INITIALIZATION_ERROR",
                     "the DynamicCommandManager is not properly initialized [no associated device]",
                     "DynamicCommandManager::add_command");
  }

  //- be sure the DynamicCommand is valid
  if (! dc || ! dc->get_tango_command() )
    return;

  //- check command does not already exist
  DynamicCommandIterator it = this->rep_.find(dc->dci_.tci.cmd_name);
  if (it != this->rep_.end())
  {
    std::ostringstream oss;
    oss << "couldn't add dynamic command <"
        << dc->dci_.tci.cmd_name
        << "> - another dynamic command already exists with same name";
    THROW_DEVFAILED("OPERATION_NOT_ALLOWED",
                    oss.str().c_str(),
                    "DynamicCommandManager::add_command");
  }

  //- add the dynamic command to the device interface
  try
  {
    std::vector< Tango::Command*>& command_list = this->dev_->get_device_class()->get_command_list();
    command_list.push_back(dc->get_tango_command());
  }
  catch (Tango::DevFailed& ex)
  {
    std::ostringstream oss;
    oss << "couldn't add dynamic command <"
        << dc->dci_.tci.cmd_name
        << "> - Tango exception caught";
    RETHROW_DEVFAILED(ex,
                      "API_ERROR",
                      oss.str().c_str(),
                      "DynamicCommandManager::add_command");
  }
  catch (...)
  {
    std::ostringstream oss;
    oss << "couldn't add dynamic command <"
        << dc->dci_.tci.cmd_name
        << "> - Tango exception caught";
    THROW_DEVFAILED("API_ERROR",
                    oss.str().c_str(),
                    "DynamicCommandManager::add_command");
  }

  //- insert the command into the local repository
  std::pair<DynamicCommandIterator, bool> insertion_result;
  insertion_result = this->rep_.insert(DynamicCommandEntry(dc->dci_.tci.cmd_name, dc));
  if (insertion_result.second == false)
  {
    THROW_DEVFAILED("INTERNAL_ERROR",
                    "failed to insert the dynamic command into the local repository",
                    "DynamicCommandManager::add_command");
  }
}

// ============================================================================
// DynamicCommandManager::remove_command
// ============================================================================
void DynamicCommandManager::remove_command (const std::string& cn)
{
  YAT4TANGO_TRACE("DynamicCommandManager::remove_command");

  if ( ! this->dev_ )
    return;

  //- command exists?
  DynamicCommandIterator it = this->rep_.find(cn);
  if (it == this->rep_.end())
  {
    THROW_DEVFAILED("ATTRIBUTE_NOT_FOUND",
                    "no such dynamic command [unknown command name]",
                    "DynamicCommandManager::add_command");
  }

  //- found... remove it from the device
  try
  {
    //- get dynamic command
    DynamicCommand * dc = (*it).second;
    //- remove it from the device commands list
    std::vector<Tango::Command*> & dcl = this->dev_->get_device_class()->get_command_list();
    std::vector<Tango::Command*>::iterator cit = dcl.begin();
    for ( ; cit != dcl.end(); ++cit )
    {
      if ( *cit == dc->get_tango_command() )
      {
        dcl.erase(cit);
        break;
      }
    }
    //- remove cmd from local repository
    this->rep_.erase(it);
    //- release associated memory
    delete dc;
  }
  catch (Tango::DevFailed& ex)
  {
    RETHROW_DEVFAILED(ex,
                      "INTERNAL_ERROR",
                      "failed to remove dynamic command from the device interface",
                      "DynamicCommandManager::remove_command");
  }
  catch (...)
  {
    THROW_DEVFAILED("UNKNOWN_ERROR",
                    "failed to remove dynamic command from the device interface",
                    "DynamicCommandManager::remove_command");
  }
};

// ============================================================================
// DynamicCommandManager::remove_commands
// ============================================================================
void DynamicCommandManager::remove_commands()
{
  YAT4TANGO_TRACE("DynamicCommandManager::remove_commands");

  if ( ! this->dev_ )
    return;

  try
  {
    //- build dynamic commands name list
    std::vector<std::string> cl;
    for ( DynamicCommandIterator it = this->rep_.begin(); it != this->rep_.end(); ++it )
      cl.push_back(it->first);

    //- remove each dynamic command
    for ( size_t i = 0; i < cl.size(); i++ )
      this->remove_command(cl[i]);
  }
  catch ( ... )
  {
    //- finally, clear the repository...
    this->rep_.clear();
    throw;
  }

  //- finally, clear the repository...
  this->rep_.clear();
}

} // namespace


