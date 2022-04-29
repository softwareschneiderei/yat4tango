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

#ifndef _FILE_APPENDER_H_
#define _FILE_APPENDER_H_

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <deque>
#include <map>
#include <tango.h>
#include <yat/threading/Mutex.h>
#include <yat/threading/Task.h>
#include <yat/time/Time.h>
#include <yat/file/FileName.h>
#include <yat/memory/UniquePtr.h>
#include <yat4tango/DynamicAttributeManager.h>

namespace yat4tango
{

// ============================================================================
//! Min available megabytes in file system before deleting oldest log fie (if any)
//! 0 mean ignore
// ============================================================================
#define FILE_APPENDER_DEFAULT_FS_MIN_AVAILABLE_MB 100

// ============================================================================
//! Checking period in seconds (in interval [10, 3600])
// ============================================================================
#define FILE_APPENDER_DEFAULT_CHECK_PERIOD 60

// ============================================================================
//! Highest possible value (Mb) for maximum total size for log messages is 100Gb
// ============================================================================
#define FILE_APPENDER_MAXVALUE_MAX_ABSOLUTE_SIZE 100000

// ============================================================================
//! Max total size for log messages (Mb)
// ============================================================================
#define FILE_APPENDER_DEFAULT_MAX_ABSOLUTE_SIZE 100

// ============================================================================
//! Highest possible value (Kb) for maximum file size is 100Mb
// ============================================================================
#define FILE_APPENDER_MAXVAL_MAX_FILE_SIZE 100000

// ============================================================================
//! Default max file size (Kb)
// ============================================================================
#define FILE_APPENDER_DEFAULT_MAX_FILE_SIZE 1000

// ============================================================================
//! Highest possible value for log buffer depth in days
// ============================================================================
#define FILE_APPENDER_MAXVAL_MAX_FILE_DAYS 90

// ============================================================================
//! Default log buffer depth in days
// ============================================================================
#define FILE_APPENDER_DEFAULT_MAX_FILE_DAYS 7

// ============================================================================
//! Default log files path
// ============================================================================
#define FILE_APPENDER_DEFAULT_PATH "/var/log/tango_devices"

// ============================================================================
//! Default DEBUG level log
// ============================================================================
#define FILE_APPENDER_DEFAULT_DEBUG_LVL true

// ============================================================================
//! \class FileAppender
//! \brief The appender for a Tango Device internal activity.
//!
//! This class provides a way to create an appender to log the internal activity
//! of a Tango Device in a cyclic buffer. This buffer is implemented as a
//! dynamic attribute of the Device (named "log", which type is SPECTRUM of strings).
//!
//! \b Usage:
//! - Initialize the appender in the device initialization function (i.e. the *init_device()*
//! function). This is the first thing to do before any other action. For instance:
//! \verbatim yat4tango::FileAppender::initialize(this); \endverbatim
//!
//! - Release the appender in the device destructor (i.e. the *delete_device()* function).
//! This is the last thing to do before removing the device. For instance:
//! \verbatim yat4tango::FileAppender::release(this); \endverbatim
//!
//! Inherits from log4tango::Appender class.
//!
// ============================================================================
class YAT4TANGO_DECL FileAppender : public log4tango::Appender
{
public:

  //! \brief Configuration
  struct Config
  {
    Tango::DeviceImpl* device_p;
    std::size_t        max_file_days;
    std::size_t        max_file_size_kb;
    yat::uint64        max_total_size_mb;
    yat::uint64        min_available_mb;
    std::string        path;
    bool               load_previous_logs;
    bool               log_debug_level;
    // Initialize default values
    Config();
  };

public:

  //! \brief Initialization of the inner appender.
  //!
  //! \param cfg configuration
  //! \exception DEVICE_ERROR Thrown when an error occurs while initializing the logger.
  static void initialize(const Config& cfg);

  //! \brief Initialization of the inner appender.
  //!
  //! \param associated_device The Tango device to log.
  //! \param max_log_depth maximum log history depth
  //! \param path logs storage path
  //! \exception DEVICE_ERROR Thrown when an error occurs while initializing the logger.
  static void initialize(Tango::DeviceImpl* associated_device,
                          std::size_t max_log_depth = FILE_APPENDER_DEFAULT_MAX_FILE_DAYS,
                          const std::string& path = FILE_APPENDER_DEFAULT_PATH);

  //! \brief Initialization of the file appender.
  //!
  //! \param associated_device The Tango device to log.
  //! \param load_previous_logs Load log messages from the previous sessions
  //! \param max_log_depth maximum log history depth
  //! \param path logs storage path
  //! \exception DEVICE_ERROR Thrown when an error occurs while initializing the logger.
  //! \warning the InnerAppender class must have been initialized first
  static void initialize(Tango::DeviceImpl* associated_device,
                         bool load_previous_logs,
                         std::size_t max_log_depth = FILE_APPENDER_DEFAULT_MAX_FILE_DAYS,
                         const std::string& path = FILE_APPENDER_DEFAULT_PATH);

  //! \brief Termination of the appender.
  //!
  //! \param associated_device The associated Tango device.
  //! \exception DEVICE_ERROR Thrown when an error occurs while releasing the logger.
  static void release(Tango::DeviceImpl* associated_device);

  //! \brief Reset the appender. Close log file then
  //! \brief archive current and previous log files(s)
  //! \brief in sub-folder '<domain>_<family>_<member>.yyyymmddHHMMSS'
  //!
  static void archive(Tango::DeviceImpl * hd);

  //! \brief Fill the inner appender with previous messages
  //!
  //! \param vec ordered messages
  static void fill_inner_appender(Tango::DeviceImpl* associated_device);

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

  //- Return true if the FileAppender class is initialized
  static bool is_initialized();

protected:
  //! \brief Log appender method.
  //! \param event The event to log.
  virtual int _append(const log4tango::LoggingEvent& event);

private:
  //- dedicate a type to the log buffer
  typedef std::deque<std::string> LogBuffer;

  //- FileAppender repository
  typedef std::map<Tango::DeviceImpl*, FileAppender*> FileAppenderRepository;
  typedef FileAppenderRepository::value_type FileAppenderEntry;
  typedef FileAppenderRepository::iterator FileAppenderIterator;
  typedef FileAppenderRepository::const_iterator FileAppenderConstIterator;

  //- provide the yat4tango::DynamicAttributeReadCallback with access to read_callback
  friend class DynamicAttributeReadCallback;

  //- provide the log4tango::Logger with access to the dtor
  friend class log4tango::Logger;

  //- Ctor
  FileAppender();

  //- Dtor
  virtual ~FileAppender();

  //- Initialization
  void initialize_i(const Config& cfg);

  //- Reset appender and delete existing log file(s)
  void reset_i();

  //- Fill the inner appender with previous messages
  void fill_inner_appender_i();

  //- the host device
  Tango::DeviceImpl* m_dev;

  // device name
  yat::String m_dev_name;

  //- Base file name
  yat::FileName m_file_name;

  //- Debug level log policy
  bool m_log_debug_level;

  //- Watcher
   yat::UniquePtr<class FileAppenderWatcher, yat::TaskExiter> m_watcher_ptr;

  //- FileAppender repository
  static FileAppenderRepository m_rep;

  //- thread safety
  static yat::Mutex m_rep_lock;

  //- disabled methods
  FileAppender (const FileAppender&);
  const FileAppender& operator= (const FileAppender&);
};

} // namespace

#endif // _FILE_APPENDER_H_
