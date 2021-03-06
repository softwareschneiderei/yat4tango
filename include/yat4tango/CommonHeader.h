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

#ifndef _YAT4TANGO_COMMON_H_
#define _YAT4TANGO_COMMON_H_

// ============================================================================
// DEPENDENCIES
// ============================================================================
#ifdef WIN32
#pragma warning( push )
#pragma warning( disable: 4786 ) // 
#pragma warning( disable: 4267 ) // 'var' : conversion from 'size_t' to 'type', possible loss of data 
#pragma warning( disable: 4311 ) // 'variable' : pointer truncation from 'type' to 'type' 
#pragma warning( disable: 4312 ) // 'operation' : conversion from 'type1' to 'type2' of greater size 
#endif

#ifdef WIN32
#pragma warning( pop )
#endif

#include <yat/CommonHeader.h>
#include <yat4tango/Export.h>
#include <tango.h>

#endif
