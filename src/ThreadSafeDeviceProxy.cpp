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

//=============================================================================
// DEPENDENCIES
//=============================================================================
#include <yat4tango/ThreadSafeDeviceProxy.h>

#if ! defined (YAT_INLINE_IMPL)
#  include "yat4tango/ThreadSafeDeviceProxy.i"
#endif

namespace yat4tango
{

//=============================================================================
// ThreadSafeDeviceProxy::ThreadSafeDeviceProxy 
//=============================================================================
ThreadSafeDeviceProxy::ThreadSafeDeviceProxy (const std::string& dev_name)
 : dp_(const_cast<std::string&>(dev_name))  
{
  //- noop ctor
}

//=============================================================================
// ThreadSafeDeviceProxy::ThreadSafeDeviceProxy 
//=============================================================================
ThreadSafeDeviceProxy::ThreadSafeDeviceProxy (const char * dev_name)
 : dp_(dev_name)  
{
  //- noop ctor
}

//=============================================================================
// ThreadSafeDeviceProxy::~ThreadSafeDeviceProxy 
//=============================================================================
ThreadSafeDeviceProxy::~ThreadSafeDeviceProxy ()
{
  //- noop dtor
}

} // namespace
