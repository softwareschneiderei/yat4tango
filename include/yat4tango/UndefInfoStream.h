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
// Copyright (C) 2006-2016 The Tango Community
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

#ifndef _YAT4TANGO_UNDEF_INFO_STREAM_H_
#define _YAT4TANGO_UNDEF_INFO_STREAM_H_

// Undefine the INFO_STREAM macro because of too abusive use by Pogo
#ifdef INFO_STREAM
  #undef INFO_STREAM
#endif

namespace yat4tango
{
  // re-define the INFO_STREAM
  struct NullInfoStream : std::ostream
  {
    NullInfoStream(): std::ios(0), std::ostream(0) {}
  };
  template <typename T>
  NullInfoStream& operator<<(NullInfoStream& ns, T) { return ns; }
}

  yat4tango::NullInfoStream g_null_info_stream;

  #define INFO_STREAM g_null_info_stream

// INFO_STREAM substitution
#define Y4T_INFO_STREAM \
  if (get_logger()->is_info_enabled()) \
    get_logger()->info_stream() << log4tango::LogInitiator::_begin_log


#endif //- _YAT4TANGO_UNDEF_INFO_STREAM_H_
