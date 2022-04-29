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

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <yat4tango/LogHelper.h>

namespace yat4tango
{

LogFormat TangoLogAdapter::debug(const std::string& msg_fmt)
{
  return LogFormat(msg_fmt, Tango::LOG_DEBUG, logger_);
}

LogFormat TangoLogAdapter::info(const std::string& msg_fmt)
{
  return LogFormat(msg_fmt, Tango::LOG_INFO, logger_);
}

LogFormat TangoLogAdapter::warn(const std::string& msg_fmt)
{
  return LogFormat(msg_fmt, Tango::LOG_WARN, logger_);
}

LogFormat TangoLogAdapter::error(const std::string& msg_fmt)
{
  return LogFormat(msg_fmt, Tango::LOG_ERROR, logger_);
}

LogFormat TangoLogAdapter::fatal(const std::string& msg_fmt)
{
  return LogFormat(msg_fmt, Tango::LOG_FATAL, logger_);
}

} // namespace


