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

#ifndef _Y4T_LOGGING_H_
#define _Y4T_LOGGING_H_

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <yat4tango/Export.h>

namespace yat4tango
{

//=============================================================================
//!
//! \class Logging
//!
//! \brief Aggregation class to ease logging management
//!
//! This class provides a simplier way to declare & use the logging classes of
//! YAT4Tango: YatLogAdapter, InnerAppender, FileAppender.
//! Based on a property content, it configure InnerAppenper & FileAppender static objects:
//! ('default values') desc
//! \verbatim
//! [device_stream]
//! attr = log                     ('log') the log attribute name
//! order = backward               ('forward') messages order in the log
//! depth = 1000                   ('2048') max number of messages in the log before rolling
//! layout = default_multi         ('default_single') multi or single column
//! [file_stream]
//! path = /var/log/tango_devices  ('/var/log/cpp_devices') root path
//! max_file_size_kb = 1000        ('1000') min file size before archiving
//! max_file_days = 90             ('5') how many days before old log files are deleted
//! debug_level = false            ('true') store debug level messages or not
//! \endverbatim
//!
//=============================================================================
class YAT4TANGO_DECL Logging
{
public:
  //! \brief Initialize this host and configuration property name
  //!
  //! \param device_p Host device
  //! \param prop_name Name of the configuration property
  static void initialize(Tango::DeviceImpl* device_p, const std::string& prop_name);
  static void initialize(Tango::DeviceImpl* device_p);
  static void release(Tango::DeviceImpl* device_p);

  //! \brief Close/rename log files and move them into a new folder named
  //!  '<domain>_<family>_<member>.yyyymmddHHMMSS'
  static void archive_and_reset(Tango::DeviceImpl* device_p);

  //! \brief Clear the log attribute
  //!
  //! Same effect as calling InnerAppender::clear()
  static void clear_log(Tango::DeviceImpl* device_p);
};

} // namespace

#endif // _LOGGING_H_
