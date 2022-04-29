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

#include <yat4tango/YatLogAdapter.h>

namespace yat4tango
{

//=============================================================================
// YatLogAdapter::initialize
//=============================================================================
void YatLogAdapter::initialize(Tango::DeviceImpl* device_p)
{
	yat::LogManager::init(yat::LOG_VERBOSE);
  instance().m_tango_log_adapter = new yat4tango::TangoLogAdapter(device_p);
	instance().m_log_catcher_p = new yat::LogCatcher(&instance());
}

//=============================================================================
// YatLogAdapter::log
//=============================================================================
void YatLogAdapter::log(yat::ELogLevel log_level, yat::pcsz log_type, const std::string& log_msg)
{
  (void)log_type;
	switch( log_level )
	{
		case yat::LOG_VERBOSE:
			m_tango_log_adapter->get_logger()->debug_stream() << log_msg << std::endl;
			break;
		case yat::LOG_RESULT:
		case yat::LOG_INFO:
		case yat::LOG_NOTICE:
			m_tango_log_adapter->get_logger()->info_stream() << log_msg << std::endl;
			break;
		case yat::LOG_WARNING:
			m_tango_log_adapter->get_logger()->warn_stream() << log_msg << std::endl;
			break;
		case yat::LOG_ERROR:
			m_tango_log_adapter->get_logger()->error_stream() << log_msg << std::endl;
			break;
		default: // >ERROR
			m_tango_log_adapter->get_logger()->fatal_stream() << log_msg << std::endl;
			break;
	}
}

//=============================================================================
// YatLogAdapter::release
//=============================================================================
void YatLogAdapter::release()
{
	delete instance().m_tango_log_adapter;
	delete instance().m_log_catcher_p;
}

} // namespace
