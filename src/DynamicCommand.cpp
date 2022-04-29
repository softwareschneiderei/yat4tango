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
#include <limits>
#include <algorithm>
#if defined(WIN32)
# include <cctype>
#endif
#include "yat/utils/StringTokenizer.h"
#include "yat4tango/DynamicCommand.h"

namespace yat4tango
{

// ============================================================================
// DynamicCommandInfo::DynamicCommandInfo
// ============================================================================
DynamicCommandInfo::DynamicCommandInfo ()
  : dev(0),
    tci(),
    ecb(0),
    uad(0)
{
  tci.cmd_name   = "";
  tci.cmd_tag    = 0;
  tci.in_type    = Tango::DEV_VOID;
  tci.out_type   = Tango::DEV_VOID;
  tci.disp_level = Tango::OPERATOR;
}

// ============================================================================
// DynamicCommandInfo::DynamicCommandInfo
// ============================================================================
DynamicCommandInfo::DynamicCommandInfo (const DynamicCommandInfo& s)
{
  *this = s;
}

// ============================================================================
// DynamicCommandInfo::operator=
// ============================================================================
const DynamicCommandInfo& DynamicCommandInfo::operator= (const DynamicCommandInfo& s)
{
  if ( &s == this )
    return *this;

  dev = s.dev;
  tci = s.tci;
  ecb = s.ecb;
  uad = s.uad;

  return *this;
}

// ============================================================================
// DynamicCommandInfo::get_tango_data_type_str
// ============================================================================
const char * DynamicCommandInfo::get_tango_data_type_str (ArgIO arg) const
{
  static const char * __str__[] =
  {
    "DEV_VOID",
    "DEV_BOOLEAN",
    "DEV_SHORT",
    "DEV_LONG",
    "DEV_FLOAT",
    "DEV_DOUBLE",
    "DEV_USHORT",
    "DEV_ULONG",
    "DEV_STRING",
    "DEVVAR_CHARARRAY",
    "DEVVAR_SHORTARRAY",
    "DEVVAR_LONGARRAY",
    "DEVVAR_FLOATARRAY",
    "DEVVAR_DOUBLEARRAY",
    "DEVVAR_USHORTARRAY",
    "DEVVAR_ULONGARRAY",
    "DEVVAR_STRINGARRAY",
    "DEVVAR_LONGSTRINGARRAY",
    "DEVVAR_DOUBLESTRINGARRAY",
    "DEV_STATE",
    "CONST_DEV_STRING",
    "DEVVAR_BOOLEANARRAY",
    "DEV_UCHAR",
    "DEV_LONG64",
    "DEV_ULONG64",
    "DEVVAR_LONG64ARRAY",
    "DEVVAR_ULONG64ARRAY",
    "DEV_INT",
    "DEV_ENCODED",
#if (TANGO_VERSION_MAJOR >= 9)
    "DEV_ENUM",
    "DEV_PIPE_BLOB",
    "DEVVAR_STATEARRAY",
#endif
    //-----------
    "UNKNOWN_TANGO_DATA_TYPE"
  };

#if (TANGO_VERSION_MAJOR >= 9)
  static int __unknown_id__ = Tango::DEVVAR_STATEARRAY + 1;
#else
  static int __unknown_id__ = Tango::DEV_ENCODED + 1;
#endif

  int i = ( arg == ARGIN )
        ? this->tci.in_type
        : this->tci.out_type;

#if (TANGO_VERSION_MAJOR >= 9)
  if (i >= Tango::DEV_VOID && i <= Tango::DEVVAR_STATEARRAY)
#else
  if (i >= Tango::DEV_VOID && i <= Tango::DEV_ENCODED)
#endif
    return __str__[i];

  return __str__[__unknown_id__];
}

// ============================================================================
// DynamicCommandInfo::dump
// ============================================================================
void DynamicCommandInfo::dump () /*const*/
{
  std::cout << "DynamicCommandInfo.this......." << this << std::endl;
  std::cout << "DynamicCommandInfo.rem_name..." << this->tci.cmd_name << std::endl;
  std::cout << "DynamicCommandInfo.in-type...." << this->get_tango_data_type_str(ARGIN) << std::endl;
  std::cout << "DynamicCommandInfo.out-type..." << this->get_tango_data_type_str(ARGOUT) << std::endl;
  std::cout << "DynamicCommandInfo.ecb........" << (this->ecb.is_empty() ? "no" : "yes") << std::endl;
  std::cout << "DynamicCommandInfo.uad........" << this->uad.type().name() << std::endl;
}

// ============================================================================
// DynamicCommand::DynamicCommand
// ============================================================================
DynamicCommand::DynamicCommand (const DynamicCommandInfo & i)
 : Tango::LogAdapter(i.dev), dci_(i), argin_(0), argout_(0)
{
  YAT4TANGO_TRACE("DynamicCommand::DynamicCommand");
}

// ============================================================================
// DynamicCommand::~DynamicCommand
// ============================================================================
DynamicCommand::~DynamicCommand ()
{
  //- no log here, crash otherwise!
}

// ============================================================================
//  DynamicCommand::set_execute_callback
// ============================================================================
void DynamicCommand::set_execute_callback (const DynamicCommandExecuteCallback & ecb)
{
  this->dci_.ecb = ecb;
}

// ============================================================================
// DynamicCommand::get_tango_data_type_str
// ============================================================================
const char * DynamicCommand::get_tango_data_type_str (ArgIO arg) const
{
  return this->dci_.get_tango_data_type_str(arg);
}

// ============================================================================
// DynamicCommand::dump
// ============================================================================
void DynamicCommand::dump () /*const*/
{
  this->dci_.dump();
}

// ============================================================================
// template class GenericCommand
// ============================================================================
class GenericCommand : public Tango::Command, public DynamicCommand
{
public:
  //---------------------------------------------------------------------------
  //- ctor
  //---------------------------------------------------------------------------
  GenericCommand (const DynamicCommandInfo& i)
    : Tango::Command (i.tci.cmd_name.c_str(),
                      static_cast<Tango::CmdArgType>(i.tci.in_type),
                      static_cast<Tango::CmdArgType>(i.tci.out_type),
                      static_cast<Tango::DispLevel>(i.tci.disp_level)),
      DynamicCommand (i)
  {
    //- noop
  }

  //---------------------------------------------------------------------------
  //- dtor
  //---------------------------------------------------------------------------
  ~GenericCommand()
  {
    //- no log here, crash otherwise!
  }

  //---------------------------------------------------------------------------
  //- returns the underlying Tango::Command
  //---------------------------------------------------------------------------
  virtual Tango::Command * get_tango_command ()
  {
    return this;
  }

  //---------------------------------------------------------------------------
  //- returns the underlying Tango data type of this DynamicCommand
  //---------------------------------------------------------------------------
  virtual int get_tango_data_type (ArgIO arg) const
  {
    return arg == ARGIN
                ? this->dci_.tci.in_type
                : this->dci_.tci.out_type;
  }

  //---------------------------------------------------------------------------
  //- called by Tango on client <execute> request
  //---------------------------------------------------------------------------
  virtual CORBA::Any* execute (Tango::DeviceImpl*, const CORBA::Any &any)
  {
    YAT4TANGO_TRACE("GenericCommand::execute");

    if ( this->dci_.ecb.is_empty() )
    {
      THROW_DEVFAILED("DEVICE_ERROR",
                      "this dynamic command has no associated <execute callback> [this is a programming error]",
                      "yat4tango::DynamicCommand::execute");
    }

    //- argIO
    this->argin_ = ( this->dci_.tci.in_type == Tango::DEV_VOID ) ? 0 : &any;
    this->argout_ = 0;

    //- callback data
    DynamicCommandExecuteCallbackData ebd;
    ebd.dev = this->dci_.dev;
    ebd.dyc = this;

    //- call execute callback
    this->dci_.ecb(ebd);

    return ( this->argout_ != 0 ) ? this->argout_ : new CORBA::Any();
  }

  //---------------------------------------------------------------------------
  //- is_allowed --------------------------------------------------------------
  //---------------------------------------------------------------------------
  virtual bool is_allowed (Tango::DeviceImpl *, const CORBA::Any &any)
  {
    return true;
  }

	//- arg-io data type
	int argin_data_type;
  int argout_data_type;
};

// ============================================================================
// DynamicCommandFactory::create_command
// ============================================================================
DynamicCommand * DynamicCommandFactory::create_command (const yat4tango::DynamicCommandInfo& i)
{
  //- no dynamic attr. named state or status
  std::string a(i.tci.cmd_name);
  std::transform(a.begin(), a.end(), a.begin(), (int(*)(int))std::tolower);

  if ( a == "state" || a == "status" )
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "can't create a dynamic command named <State> or <Status>",
                    "yat4tango::DynamicCommandFactory::create_command");
  }

  //- instanciate
  DynamicCommand * dc = 0;

  try
  {
    dc = new GenericCommand(i);
  }
  catch (Tango::DevFailed& df)
  {
    delete dc;
    RETHROW_DEVFAILED(df,
                      "DEVICE_ERROR",
                      "DynamicCommand allocation failed [Tango exception caught]",
                      "DynamicCommandFactory::create_command");
  }
  catch (...)
  {
    delete dc;
    THROW_DEVFAILED("INTERNAL_ERROR",
                    "DynamicCommand allocation failed [unknown exception caught]",
                    "DynamicCommandFactory::create_command");
  }

  if ( ! dc )
  {
    THROW_DEVFAILED("MEMORY_ERROR",
                    "DynamicCommand allocation failed",
                    "DynamicCommandFactory::create_command");
  }

  return dc;
}

} //- namespace
