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


//=============================================================================
// Usage:
//  * declare a ArchEventPusher object
//  * configure list of attributes associated to archive events
//  * launch the device task
//  * post message when event is to be sent (data & date)
//  * get errors periodically
//
//  Example:
//  // configure event sender
//  ArchiveEventUtils::ArchEventPusher::Config cfg;
//  cfg.host = m_oHostDevice;
//  cfg.retry_number = 5;
//  cfg.retry_delta = 3000;
//  m_archSender = new ArchiveEventUtils::ArchEventPusher(cfg);
//
//  // add list of tango attributes
//  std::vector<std::string> attrs;
//  attrs.push_back("xxxx");
//  attrs.push_back("yyyy");
//  m_archSender->configure(attrs);
//  m_archSender->go();
//
//  // ...
//
//  // time to send an event for xxxx attribute
//  ArchiveEventUtils::ArchEventPusher::DataToPush dtp;
//  dtp.attr_name = "xxxx";
//  dtp.data_to_send = my_data;
//  dtp.tv.tv_sec = (time_t)(my_time_sec);
//  dtp.tv.tv_usec = (suseconds_t)(my_time_usec);
//  try
//  {
//    yat::Message * msg = yat::Message::allocate(ARCH_DATA, DEFAULT_MSG_PRIORITY, false);
//    msg->attach_data(dtp);
//    m_archSender->post(msg);
//  }
//  catch (...)
//  {
//    ERROR_STREAM << "Archive event emission in error!!" << std::endl;
//  }
//
//  // ...
//
//  // get event pusher errors
//  std::string errs = "";
//  bool err_flag = m_archSender->get_errors(errs);
//  if (err_flag)
//  {
//    WARN_STREAM << "Archive event sender error: " << errs << std::endl;
//    m_strStatus += errs;
//  }
//
//=============================================================================

#include <yat/CommonHeader.h>
#if !defined (YAT_WIN32)

#ifndef _YAT4TANGO_ARCH_EVENT_PUSHER_H_
#define _YAT4TANGO_ARCH_EVENT_PUSHER_H_

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <yat/utils/String.h>
#include <yat/time/Time.h>
#include <yat/memory/SharedPtr.h>
#include <yat4tango/DeviceTask.h>

namespace yat4tango
{

// ============================================================================
// SOME USER DEFINED MESSAGES FOR THE Archive event pusher task
// ============================================================================
#define ARCH_DATA     (yat::FIRST_USER_MSG + 1001)
#define ARCH_ERR_MSG  (yat::FIRST_USER_MSG + 1002)
#define FIRST_PRIV_MSG (yat::FIRST_USER_MSG + 2000)
#define PRIV_SEND_DATA (FIRST_PRIV_MSG + 1)

#define ERROR_MSG_WAIT 1000 // ms

// ============================================================================
//! \class ArchEventPusher
//! \brief Read message queue requests and transforms them in archive event.
//!
//! \note This class is not implemented yet on Windows plateform
// ============================================================================
class ArchEventPusher : public yat4tango::DeviceTask
{
public:

  //! \brief Structure for pusher configuration
  typedef struct Config
  {
    //! device host
    Tango::DeviceImpl * host;

    //! number of retry
    size_t retry_number;

    //! time between retries, in ms
    yat::uint32 retry_delta;

    // default values
    // 16 * 250ms = 4s (at least)
    Config() : host(NULL), retry_number(16), retry_delta(250) {}
  } Config;

  //! \brief structure for data to push as archive event
  typedef struct DataToPush
  {
    //! attribute name
    std::string attr_name;

    //! data
    double data_to_send;

    //! timestamp
    timeval tv;

  } DataToPush;

  //! Shortcut for shared pointer on DataToPush structure
  typedef YAT_SHARED_PTR(DataToPush) DataToPushPtr;

  //! \brief Structure for error message
  typedef struct ErrorMsg
  {
    //! error flag
    bool flag;

    //! error message
    std::string message;
  } ErrorMsg;

  //! \brief Constructor
  ArchEventPusher (Config cfg)
   :yat4tango::DeviceTask(cfg.host),
    m_config(cfg), m_attempts_count(0)

  {
    m_error_msg = "";
    m_error_flag = false;
  }

  //! \brief Destructor
  ~ArchEventPusher ()
  {}

  //! \brief Configure manual fire of archive event for the specified list of attribute names
  void configure(std::vector<std::string> attrs);

  //! \brief get error message
  bool get_errors(std::string & error_msg);

protected:

  void priv_send_data();

  // process message queue messages
  void process_message(yat::Message& msg);

private:
  // Pusher configuration
  Config m_config;

  // error management
  std::size_t m_attempts_count;
  std::string m_error_msg;
  bool m_error_flag;
  std::size_t n_retry;
  yat::Timer m_since_last_attempt;
  std::vector<DataToPushPtr> m_pending_data;
};


} //- namespace

#endif //- _YAT4TANGO_ARCH_EVENT_PUSHER_H_

#endif // YAT_WIN32
