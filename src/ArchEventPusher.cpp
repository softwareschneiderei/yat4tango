//----------------------------------------------------------------------------
// Copyright (c) 2004-2021 The Tango Community
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Lesser Public License v3
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/lgpl.html
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// YAT4Tango LIBRARY
//----------------------------------------------------------------------------
//
// Copyright (c) 2004-2021 The Tango Community
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


// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <yat/utils/Logging.h>
#include <yat4tango/ArchEventPusher.h>

#if !defined (YAT_WIN32)

namespace yat4tango
{

// ============================================================================
// ArchEventPusher::configure
// ============================================================================
void ArchEventPusher::configure(std::vector<std::string> attrs)
{
  if (!m_config.host)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
        "Cannot configure events: host is NULL!",
        "ArchEventPusher::Configure");
  }

  if ((m_config.retry_number !=0) && (m_config.retry_delta == 0))
  {
    THROW_DEVFAILED("CONFIGURATION_ERROR",
        "Retry delta is null!",
        "ArchEventPusher::Configure");
  }
  set_periodic_msg_period(m_config.retry_delta / 2);
  enable_periodic_msg(true);

  if (attrs.empty())
  {
    THROW_DEVFAILED("CONFIGURATION_ERROR",
        "Cannot configure events: attribute list is empty",
        "ArchEventPusher::Configure");
  }

  try
  {
    // define archiving event for each attribute in the list
    for (size_t idx = 0; idx < attrs.size(); idx++)
      m_config.host->set_archive_event(attrs[idx], true, false);
  }
  catch (Tango::DevFailed &e)
  {
    ERROR_STREAM << e << std::endl;
    RETHROW_DEVFAILED(e,
      "DEVICE_ERROR",
      "Cannot configure archive event!",
      "ArchEventPusher::Configure");
  }
  catch (...)
  {
    ERROR_STREAM << "Cannot configure archive event, caught [...]!" << std::endl;
    THROW_DEVFAILED(
      "DEVICE_ERROR",
      "Cannot configure archive event!",
      "ArchEventPusher::Configure");
  }
}

// ============================================================================
// ArchEventPusher::get_errors
// ============================================================================
bool ArchEventPusher::get_errors(std::string & error_msg)
{
  yat::Message* msg = yat::Message::allocate(ARCH_ERR_MSG, DEFAULT_MSG_PRIORITY, true);

  try
  {
    this->wait_msg_handled(msg->duplicate(), ERROR_MSG_WAIT);
  }
  catch (...)
  {
    msg->release();
    error_msg = "cannot get error msg from Archive Event Pusher!";
    return true;
  }

  ErrorMsg em = msg->get_data<ErrorMsg>();
  msg->release();

  error_msg = em.message;
  return em.flag;
}

// ============================================================================
// ArchEventPusher::priv_send_data
// ============================================================================
void ArchEventPusher::priv_send_data()
{
  YAT_TRACE("yat4tango::ArchEventPusher::priv_send_data");
  DataToPushPtr dtp_ptr = m_pending_data.front();
  m_attempts_count++;

  /*
  YAT_FREQUENCY_LIMITED_STATEMENT
  (
    YAT_INFO << yat::StringFormat("Push archive event for: {} - data: {} - date s: {}. attempt: {}")
                   .format(dtp_ptr->attr_name)
                   .format(dtp_ptr->data_to_send)
                   .format(dtp_ptr->tv.tv_sec)
                   .format(m_attempts_count)
                << YAT_EOL,
    1
  ); */

  try
  {
    m_config.host->push_archive_event(dtp_ptr->attr_name, &(dtp_ptr->data_to_send),
                                      dtp_ptr->tv, Tango::ATTR_VALID);
    m_since_last_attempt.restart();
    m_attempts_count = 0; // success
    m_pending_data.erase(m_pending_data.begin()); // remove pushed data
    // do not wait for the next data, if any...
    if( m_pending_data.size() > 0 )
      post(PRIV_SEND_DATA);
  }
  catch(...)
  {
    m_error_flag = true;
    m_since_last_attempt.restart();
    if( m_config.retry_number + 1 > m_attempts_count )
    {
      m_error_msg = yat::StringFormat("Push event failed for {}. {} attemtps left.")
                    .format(dtp_ptr->attr_name)
                    .format(m_config.retry_number - m_attempts_count);

      YAT_WARNING << m_error_msg << YAT_EOL;
    }
    else
    {
      m_error_msg = yat::StringFormat("Cannot send event for attribute {}. All attempts failed. Drop data.")
                   .format(dtp_ptr->attr_name);
      YAT_ERROR << m_error_msg << YAT_EOL;
      m_attempts_count = 0;
      m_pending_data.erase(m_pending_data.begin()); // drop data
    }
  }
}

// ============================================================================
// ArchEventPusher::process_message
// ============================================================================
void ArchEventPusher::process_message(yat::Message& msg)
{
  //- handle msg
  switch (msg.type())
  {
    //- THREAD_INIT ----------------------
    case yat::TASK_INIT:
    {
      //noop
    }
    break;

    //- THREAD_EXIT ----------------------
    case yat::TASK_EXIT:
    {
      // noop
    }
    break;

    //- THREAD_PERIODIC ------------------
    case yat::TASK_PERIODIC:
    {
      if( m_pending_data.size() > 0 &&
          m_since_last_attempt.elapsed_msec() >= m_config.retry_delta )
      {
        priv_send_data();
        /*
        YAT_FREQUENCY_LIMITED_STATEMENT
        ( YAT_LOG("yat4tango::ArchEventPusher: m_pending_data.size() [1]: " << m_pending_data.size()),
          1
        ); */
      }
    }
    break;


    //- THREAD_TIMEOUT -------------------
    case yat::TASK_TIMEOUT:
    {
      // noop
    }
    break;

    //- PRIV_SEND_DATA ------------------------
    case PRIV_SEND_DATA:
    {
      if( m_pending_data.size() > 0 )
        priv_send_data();
        /*
        YAT_FREQUENCY_LIMITED_STATEMENT
        ( YAT_LOG("yat4tango::ArchEventPusher: m_pending_data.size() [2]: " << m_pending_data.size()),
          1
        ); */
    }
    break;

    //- ARCH_DATA ------------------------
    case ARCH_DATA:
    {
      DataToPushPtr dtp_ptr = msg.get_data<DataToPushPtr>();
      m_pending_data.push_back(dtp_ptr);
      if( m_pending_data.size() == 1 )
        priv_send_data();
      // else nothing because a previous data push was in error and is waiting for next attempt
    }
    break;

    //- ARCH_ERR_MSG ------------------------
    case ARCH_ERR_MSG:
    {
      //DEBUG_STREAM << "ArchEventPusher::process_message::ARCH_ERR_MSG" << std::endl;
      ErrorMsg em;
      em.message = m_error_msg;
      em.flag = m_error_flag;
      msg.attach_data(em);
      m_error_flag = false; // reset error flag & msg once read
      m_error_msg = "";
    }
    break;

    //- UNHANDLED MSG --------------------
    default:
    {
      ERROR_STREAM << "ArchEventPusher::process_message::unhandled msg type received" << std::endl;
    }
    break;
  }
}

} //- namespace

#endif // YAT_WIN32
