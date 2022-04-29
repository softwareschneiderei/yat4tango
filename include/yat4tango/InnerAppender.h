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

#ifndef _INNER_APPENDER_H_
#define _INNER_APPENDER_H_

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <deque>
#include <map>
#include <tango.h>
#include <yat/threading/Mutex.h>
#include <yat/utils/String.h>
#include <yat/memory/SharedPtr.h>
#include <yat4tango/DynamicAttributeManager.h>

namespace yat4tango
{

// ============================================================================
//! Default log buffer depth
// ============================================================================
const std::size_t INNER_APPENDER_DEFAULT_BUFFER_DEPTH = 2048;
// ============================================================================
//! Default attribute name
// ============================================================================
const std::string INNER_APPENDER_DEFAULT_ATTR_NAME = "log";
// ============================================================================
//! Default layout
// ============================================================================
const std::string INNER_APPENDER_DEFAULT_LAYOUT = "default_single";
// ============================================================================
//! Default messages order
// ============================================================================
const std::string INNER_APPENDER_DEFAULT_ORDER = "forward";
// ============================================================================
//! Default user mode is 'OPERATOR'
// ============================================================================
const bool INNER_APPENDER_DEFAULT_EXPERT_FLAG = false;

// ============================================================================
//! \class InnerAppender
//! \brief The appender for a Tango Device internal activity.
//!
//! This class provides a way to create an appender to log the internal activity
//! of a Tango Device in a cyclic buffer. This buffer is implemented as a
//! dynamic attribute of the Device (named "log", which type is SPECTRUM of strings).
//!
//! \b Usage:
//! - Initialize the appender in the device initialization function (i.e. the *init_device()*
//! function). This is the first thing to do before any other action. For instance:
//! \verbatim yat4tango::InnerAppender::initialize(this); \endverbatim
//!
//! - Release the appender in the device destructor (i.e. the *delete_device()* function).
//! This is the last thing to do before removing the device. For instance:
//! \verbatim yat4tango::InnerAppender::release(this); \endverbatim
//!
//! Inherits from log4tango::Appender class.
//!
// ============================================================================
class YAT4TANGO_DECL InnerAppender : public log4tango::Appender
{
public:

  typedef enum
  {
    no_def,     // Not defined, default internal value
    default_single, // The default value
    default_multi,
    date,
    level,
    message,    // Mandatory, except if all_in_one is defined
    thread_id,
    logger_name
  } ColType;

  typedef enum
  {
    asc, reverse_order
  } Order;

  struct Config
  {
    std::vector<ColType> columns;
    std::size_t max_log_buffer_depth;
    std::string attribute_name;
    bool reverse_order;
    bool expert;

    Config(ColType col1 = default_single, ColType col2 = no_def,
           ColType col3 = no_def, ColType col4 = no_def, ColType col5 = no_def);
    void set_columns(const yat::String& columns);
    void set_order(const std::string& order_spec);
  };

  struct Log
  {
    std::string level;
    std::string date;
    std::string message;
    std::string thread_id;
    std::string logger_name;
    std::string full_message;
  };
  typedef yat::SharedPtr<Log> LogPtr;

  //! \brief Initialization of the inner appender.
  //!
  //! \param associated_device The Tango device to log.
  //! \param max_log_buffer_depth
  //! \exception DEVICE_ERROR Thrown when an error occurs while initializing the logger.
  static void initialize(Tango::DeviceImpl* associated_device,
                         std::size_t max_log_buffer_depth = INNER_APPENDER_DEFAULT_BUFFER_DEPTH,
                         const std::string & attribute_name = INNER_APPENDER_DEFAULT_ATTR_NAME,
                         Order o = asc);

  //! \brief Initialization of the inner appender.
  //!
  //! \param associated_device The Tango device to log.
  //! \param cfg the configuration
  //! \exception DEVICE_ERROR Thrown when an error occurs while initializing the logger.
  static void initialize(Tango::DeviceImpl* associated_device, const Config& cfg, bool no_prop=false);

  //! \brief Initialization of the inner appender. Use this signature to
  //! set reverse order
  //!
  //! \param associated_device The Tango device to log.
  //! \param cfg the configuration
  //! \param o the message ordering (reverse_order)
  //! \exception DEVICE_ERROR Thrown when an error occurs while initializing the logger.
  static void initialize(Tango::DeviceImpl* associated_device, Order o);

  //! \brief Clear messages
  static void clear(Tango::DeviceImpl* associated_device);

  //! \brief Termination of the inner appender.
  //!
  //! \param associated_device The associated Tango device.
  //! \exception DEVICE_ERROR Thrown when an error occurs while releasing the logger.
  static void release(Tango::DeviceImpl* associated_device);

  //! \brief fill log buffer with messages from a previous session
  //!
  //! \param messages input buffer
  static void fill(Tango::DeviceImpl* associated_device, std::deque<std::string>& messages);

  //! \brief get the buffer max depth value
  //!
  //! \param associated_device The associated Tango device.
  //! \exception DEVICE_ERROR Thrown if no InnerAppender is associated with the device
  static std::size_t get_max_buffer_depth(Tango::DeviceImpl* associated_device);

  //! \brief Always returns false!
  //!
  //! Inherited from log4tango::Appender virtual interface.
  virtual bool requires_layout() const;

  //! \brief Does nothing else than deleting the specified layout!
  //!
  //! Inherited from log4tango::Appender virtual interface.
  //! \param layout The layout to set.
  virtual void set_layout(log4tango::Layout* layout);

  //! \brief Does nothing!
  //!
  //! Inherited from log4tango::Appender virtual interface.
  virtual void close();

protected:
  //! \brief Log appender method.
  //! \param event The event to log.
  virtual int _append(const log4tango::LoggingEvent& event);

private:
  //- Log buffer
  typedef std::deque<LogPtr> LogBuffer;

  //- InnerAppender repository
  typedef std::map<Tango::DeviceImpl*, InnerAppender*> InnerAppenderRepository;
  typedef InnerAppenderRepository::value_type InnerAppenderEntry;
  typedef InnerAppenderRepository::iterator InnerAppenderIterator;
  typedef InnerAppenderRepository::const_iterator InnerAppenderConstIterator;

  //- provide the yat4tango::DynamicAttributeReadCallback with access to read_callback
  friend class DynamicAttributeReadCallback;

  //- provide the log4tango::Logger with access to the dtor
  friend class log4tango::Logger;

  //- Ctor
  InnerAppender ();

  //- Dtor
  virtual ~InnerAppender();

  //- Initialization
  void initialize_i(Tango::DeviceImpl* host_device,
                     const Config& cfg, bool no_prop);

  //- Clear logs buffer
  void clear_i();

  //- Termination
  void release_i();

  //! \brief fill log buffer with messages from a previous session
  //!
  //! \param messages input buffer
  void fill_i(std::deque<std::string>& messages);

  //- Load configuration from the 'Log' property value
  void prop_to_config_i(const std::vector<std::string>& prop_value);

  //- Reads callback of the underlying dynamic attribute
  void read_callback(DynamicAttributeReadCallbackData& cbd);

  //- the host device
  Tango::DeviceImpl* m_dev;

  //- dynamic attribute manager
  DynamicAttributeManager* m_dam;

  //- Configuration
  Config m_cfg;

  //- thread safety
  yat::Mutex m_lock;

  //- log buffer
  LogBuffer m_log_buffer;

  //- Messages to be deleted are transfered into this buffer until m_log_array
  // no longer reference them
  LogBuffer m_log_trash_buffer;

  //- log buffer content changed flag
  bool m_log_buffer_changed;

  // Tango array
  Tango::DevVarStringArray m_log_array;

  //- InnerAppender repository
  static InnerAppenderRepository m_rep;

  //- thread safety
  static yat::Mutex m_rep_lock;

  //- disabled methods
  InnerAppender(const InnerAppender&);
  const InnerAppender& operator=(const InnerAppender&);
};

} // namespace

#endif // _INNER_APPENDER_H_
