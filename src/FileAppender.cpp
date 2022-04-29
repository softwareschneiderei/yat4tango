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
#include <iomanip>
#include <time.h>
#include <yat/threading/Mutex.h>
#include <yat/threading/Task.h>
#include <yat/file/FileName.h>
#include <yat/utils/StringTemplate.h>
#include <yat4tango/FileAppender.h>
#include <yat4tango/InnerAppender.h>

namespace yat4tango
{

const std::string CURRENT_LOG_FILE_DATE = ".log_file_date_";

//=============================================================================
// class FileAppenderWatcher
//=============================================================================
class FileAppenderWatcher : public yat::Task
{
public:
  FileAppenderWatcher(const yat::FileName& fn,
                      std::size_t max_file_days, std::size_t max_file_size_kb,
                      std::size_t max_total_size_mb,
                      std::size_t min_available_mb,
                      yat::Mutex* file_mtx_p) : m_file_name(fn)
  {
    m_dir_pattern.assign("*/").append(fn.name()).append(".*");
    m_file_pattern.assign(fn.name()).append(".*");
    m_closed_file_pattern.assign(fn.name()).append(".log.*");

    m_max_file_days = max_file_days;
    if( m_max_file_days > FILE_APPENDER_MAXVAL_MAX_FILE_DAYS )
      m_max_file_days = FILE_APPENDER_MAXVAL_MAX_FILE_DAYS;

    m_max_file_size = 1000 * max_file_size_kb;
    if( max_file_size_kb > FILE_APPENDER_MAXVAL_MAX_FILE_SIZE )
      m_max_file_size = 1000 * FILE_APPENDER_MAXVAL_MAX_FILE_SIZE;

    m_max_total_size = 1000000LL * (yat::uint64)max_total_size_mb;
    if( max_total_size_mb > FILE_APPENDER_MAXVALUE_MAX_ABSOLUTE_SIZE )
      m_max_total_size = 1000000LL * FILE_APPENDER_MAXVALUE_MAX_ABSOLUTE_SIZE;

    if( m_max_total_size < 3 * m_max_file_size )
      m_max_file_size = m_max_total_size / 3;

#ifdef YAT_LINUX
    m_min_available_bytes = min_available_mb * 1000000; // mbytes -> bytes
#else
    m_min_available_bytes = 0; // Ignored on other platforms
#endif

    m_mtx_p = file_mtx_p;

    try
    {
      // Look for current log file date
      m_last_file_creation_date.set_empty_unix();
      yat::File f(m_file_name.path(),
                  yat::String(CURRENT_LOG_FILE_DATE).append(m_file_name.name()));
      yat::String content;
      f.load(&content);
      long unix_time = content.to_num<long>();
      m_last_file_creation_date.set_long_unix(unix_time);
    }
    catch(...) {} // don't care
  }

  void new_file()
  {
    m_last_file_creation_date = yat::CurrentUTime();
    try
    {
      yat::File f(m_file_name.path(),
                  yat::String(CURRENT_LOG_FILE_DATE).append(m_file_name.name()));
      std::ostringstream oss;
      oss << m_last_file_creation_date.long_unix() << '\n';
      f.write(oss.str());
    }
    catch(...) {} // don't care
  }

  void check_file()
  {
    if( m_file_name.file_exist() )
    {
      double age = yat::Duration(yat::CurrentTime(), m_last_file_creation_date).total_secs();
      yat::uint64 file_size = m_file_name.size64();

      if( file_size > m_max_file_size && age > SEC_PER_MIN )
      {
        rename_file();
      }
    }
  }

  void rename_file()
  {
    yat::Time tm;
    yat::DateFields df;

    yat::AutoMutex<yat::Mutex> _lock(*m_mtx_p);
    yat::FileName fn(m_file_name);
    if( fn.file_exist() )
    {
      m_file_name.mod_time(&tm);
      tm.get(&df);

      yat::FileName dest(m_file_name.path(),
                         yat::StringUtil::str_format("%s.log.%04d%02d%02d%02d%02d",
                                                     m_file_name.name().c_str(),
                                                     df.year, df.month, df.day,
                                                     df.hour, df.min));
      {
        if( !dest.file_exist() )
          fn.rename(dest.full_name());
      }
    }
  }

 void get_oldest_file_and_total_size(const yat::String& path, yat::String* oldest_file_p, double* max_age_p, yat::uint64* size_p)
  {
    yat::CurrentUTime tm_cur;
    yat::Time tm_file;
    yat::String full_name;
    // walk over current files
    yat::FileEnum fe_files(path, yat::FileEnum::ENUM_FILE);
    while( fe_files.find() )
    {
      if( yat::StringUtil::match(fe_files.name_ext(), m_closed_file_pattern) )
      {
        fe_files.mod_time(&tm_file);
        double age = yat::Duration(tm_file, tm_cur).total_secs();
        yat::uint64 size = fe_files.size64();
        *size_p += size;
        if( age > *max_age_p )
        {
          *max_age_p = age;
          *oldest_file_p = fe_files.full_name();
        }
      }
    }
  }

  yat::String get_oldest_file_and_total_size(yat::uint64* size_p)
  {
    double max_age = 0;
    yat::Time tm_file;
    *size_p = 0;

    // Get oldest file from current log path
    yat::String oldest_file;
    get_oldest_file_and_total_size(m_file_name.path(), &oldest_file, &max_age, size_p);

    // Get oldest file from log archive paths
    yat::FileEnum fe_dir(m_file_name.path(), yat::FileEnum::ENUM_DIR);
    while( fe_dir.find() )
    {
      if( yat::StringUtil::match(fe_dir.full_name(), m_dir_pattern) )
      {
        get_oldest_file_and_total_size(fe_dir.path(), &oldest_file, &max_age, size_p);
      }
    }

    return oldest_file;
  }

  void cleanup_by_size()
  {
    yat::uint64 total_size = 0, current_file_size = 0;
    if( m_file_name.file_exist() )
      current_file_size = m_file_name.size64();

    yat::FileName fn(m_file_name);

    while( true )
    {
      yat::uint64 available_bytes = 0;

      if( m_min_available_bytes > 0  )
      {
        struct yat::FileName::FSStat fs_stat = fn.file_system_statistics();
        available_bytes = fs_stat.avail;
      }

      // While total files size exceed the absolute maximum, seach and delete the oldest file
      // but only if it's not te current log file
      yat::String oldest_file = get_oldest_file_and_total_size(&total_size);
      if( (current_file_size + total_size > m_max_total_size ||
           (m_min_available_bytes > 0 && available_bytes < m_min_available_bytes)
          ) && !oldest_file.empty() )
      {
        yat::FileName fn(oldest_file);
        yat::uint64 s = fn.size64();
        try
        {
          fn.remove();
          total_size -= s;
          if( current_file_size + total_size <= m_max_total_size )
            break;
        }
        catch(...)
        { // retry later...
          break;
        }
      }
      else
        break;
    }
  }

  void cleanup_by_age()
  {
    YAT_TRACE("FileAppenderWatcher::cleanup");
    yat::FileEnum fe_files(m_file_name.path(), yat::FileEnum::ENUM_FILE);
    std::vector<std::string> files_to_delete;
    while( fe_files.find() )
    {
      if( yat::StringUtil::match(fe_files.name_ext(), m_closed_file_pattern) )
      {
        yat::CurrentTime tm_cur;
        yat::Time tm_file;
        fe_files.mod_time(&tm_file, true);
        double age = yat::Duration(tm_file, yat::CurrentUTime()).total_secs();
        if( age > m_max_file_days * 86400 )
          files_to_delete.push_back(fe_files.full_name());
      }
    }
    {
      yat::AutoMutex<yat::Mutex> _lock(*m_mtx_p);
      for( std::size_t i = 0; i < files_to_delete.size(); ++i )
      {
        yat::FileName fn(files_to_delete[i]);
        fn.remove();
      }
    }
    yat::FileEnum fe_dir(m_file_name.path(), yat::FileEnum::ENUM_DIR);
    std::vector<std::string> dir_to_delete;
    while( fe_dir.find() )
    {
      if( yat::StringUtil::match(fe_dir.full_name(), m_dir_pattern) )
      {
        yat::Time tm;
        fe_dir.mod_time(&tm);
        double age = yat::Duration(tm, yat::CurrentUTime()).total_secs();
        if( age > m_max_file_days * 86400 )
          dir_to_delete.push_back(fe_dir.path());
        // Old & empty directory ?
        else if( age > 60 ) // at least 1 minute
        {
          bool empty = true;
          yat::FileEnum fe(fe_dir.path(), yat::FileEnum::ENUM_FILE);
          while( fe.find() )
          {
            if( yat::StringUtil::match(fe.name_ext(), m_closed_file_pattern) )
            {
              empty = false;
              break;
            }
          }
          if( empty )
            dir_to_delete.push_back(fe_dir.path());
        }

      }
    }
    {
      yat::AutoMutex<yat::Mutex> _lock(*m_mtx_p);
      for( std::size_t i = 0; i < dir_to_delete.size(); ++i )
      {
        yat::FileName fn(dir_to_delete[i]);
        fn.rmdir();
      }
    }
  }

  void arch()
  {
    yat::AutoMutex<yat::Mutex> _lock(*m_mtx_p);

    rename_file();

    YAT_TRACE("FileAppenderWatcher::arch");
    yat::CurrentUTime tm;
    yat::String folder = yat::String(m_file_name.name())
                         .append(1, '.')
                         .append(yat::StringUtil::str_format("%02d%02d%02d%02d%02d%02d",
                                 tm.year(), tm.month(), tm.day(),
                                 tm.hour(), tm.minute(), int(tm.second())));

    yat::FileName fn_dst(m_file_name.path());
    fn_dst.join(folder);
    if( !fn_dst.path_exist() )
      fn_dst.mkdir();

    yat::FileEnum fe(m_file_name.path(), yat::FileEnum::ENUM_FILE);
    while( fe.find() )
    {
      if( yat::String(fe.name()).is_equal(m_file_name.name()) ||
          yat::String(fe.name_ext()).match(m_closed_file_pattern) )
        fe.move(fn_dst.path());
    }
  }

protected:
  void handle_message (yat::Message& msg)
  {
    try
    {
      switch( msg.type() )
      {
        case yat::TASK_INIT:
          set_periodic_msg_period(1000 * SEC_PER_MIN); // every minute
          enable_periodic_msg(true);
          post(yat::FIRST_USER_MSG);
          break;
        case yat::TASK_EXIT:
          break;
        case yat::TASK_PERIODIC:
        case yat::FIRST_USER_MSG:
          cleanup_by_age();
          cleanup_by_size();
        default:
          break;
      }
    }
    catch(...) {} // silently drop exceptions...
  }

private:

  yat::FileName m_file_name;
  std::string   m_closed_file_pattern;
  std::string   m_file_pattern;
  std::string   m_dir_pattern;
  yat::Mutex*   m_mtx_p;
  std::size_t   m_max_file_days;
  std::size_t   m_max_file_size;
  yat::uint64   m_max_total_size;
  yat::uint64   m_min_available_bytes;
  yat::Time     m_last_file_creation_date;
};

// ============================================================================
// FileAppender configuration
// ============================================================================
FileAppender::Config::Config()
{
  device_p = 0;
  max_file_days = FILE_APPENDER_DEFAULT_MAX_FILE_DAYS;
  max_file_size_kb = FILE_APPENDER_DEFAULT_MAX_FILE_SIZE;
  max_total_size_mb = FILE_APPENDER_DEFAULT_MAX_ABSOLUTE_SIZE;
  min_available_mb = FILE_APPENDER_DEFAULT_FS_MIN_AVAILABLE_MB;
  path = FILE_APPENDER_DEFAULT_PATH;
  load_previous_logs = true;
  log_debug_level = false;
}

// ============================================================================
// FileAppender static members
// ============================================================================
yat::Mutex FileAppender::m_rep_lock;
FileAppender::FileAppenderRepository FileAppender::m_rep;

// ============================================================================
// FileAppender::initialize
// ============================================================================
void FileAppender::initialize(Tango::DeviceImpl* hd, std::size_t max_file_days,
                              const std::string& path)
{
  initialize(hd, true, max_file_days, path);
}

// ============================================================================
// FileAppender::initialize
// ============================================================================
void FileAppender::initialize(Tango::DeviceImpl* hd, bool load_previous_logs,
                              std::size_t max_file_days, const std::string& path)
{
  Config cfg;
  cfg.device_p = hd;
  cfg.max_file_days = max_file_days;
  cfg.path = path;
  cfg.load_previous_logs = load_previous_logs;

  initialize(cfg);
}

// ============================================================================
// FileAppender::initialize
// ============================================================================
void FileAppender::initialize(const Config& cfg)
{
  yat::MutexLock guard(FileAppender::m_rep_lock);

  Tango::DeviceImpl* hd = cfg.device_p;

  if( !hd )
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "unexpected null pointer to Tango device!",
                    "yat4tango::FileAppender::initialize");
  }

  //- do we already have an FileAppender registered for the specified device?
  FileAppenderIterator it = FileAppender::m_rep.find(hd);
  if( it != FileAppender::m_rep.end() )
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "an FileAppender is already associated to the specified device",
                    "FileAppender::initialize");
  }

  FileAppender* fa = new (std::nothrow) FileAppender;
  if( !fa )
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "yat4tango::FileAppender instanciation failed!",
                    "yat4tango::FileAppender::initialize");
  }

  try
  {
    fa->initialize_i(cfg);
  }
  catch(yat::Exception& e)
  {
    e.push_error("ERROR", "Unable to initialize YAT4Tango::FileAppender!!!", "FileAppender::initialize");
    THROW_YAT_TO_TANGO_EXCEPTION(e);
  }

  //- insert the FileAppender into the local repository
  std::pair<FileAppenderIterator, bool> insertion_result;
  insertion_result = FileAppender::m_rep.insert(FileAppenderEntry(hd, fa));
  if (insertion_result.second == false)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "failed to insert the FileAppender into the local repository",
                    "yat4tango::FileAppender::initialize");
  }

  if( cfg.load_previous_logs )
    fill_inner_appender(hd);
}

// ============================================================================
// FileAppender::is_initialized
// ============================================================================
bool FileAppender::is_initialized()
{
  if( FileAppender::m_rep.size() > 0 )
    return true;
  return false;
}

// ============================================================================
// FileAppender::archive
// ============================================================================
void FileAppender::archive(Tango::DeviceImpl * hd)
{
  FileAppenderIterator it = FileAppender::m_rep.find(hd);
  if( FileAppender::m_rep.end() == it )
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "Can't archive. No FileAppender associated with the requested device",
                    "FileAppender::archive");
  }
  it->second->m_watcher_ptr->arch();
}

// ============================================================================
// FileAppender::release
// ============================================================================
void FileAppender::release(Tango::DeviceImpl * hd)
{
  yat::MutexLock guard(FileAppender::m_rep_lock);

  if (! hd)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "unexpected null pointer to Tango device!",
                    "yat4tango::FileAppender::release");
  }

  //- do we have an FileAppender registered for the specified device?
  FileAppenderIterator it = FileAppender::m_rep.find(hd);
  if (it != FileAppender::m_rep.end())
  {
    hd->get_logger()->remove_appender(it->second);
    //- remove the FileAppender from the local repository
    FileAppender::m_rep.erase(it);
  }
}

// ============================================================================
// FileAppender::fill_inner_appender
// ============================================================================
void FileAppender::fill_inner_appender(Tango::DeviceImpl* hd)
{
  //- do we have an FileAppender registered for the specified device?
  FileAppenderIterator it = FileAppender::m_rep.find(hd);
  if (it == FileAppender::m_rep.end())
    THROW_DEVFAILED("DEVICE_ERROR",
                    "No FileAppender registered for this device!",
                    "yat4tango::FileAppender::reset");

  it->second->fill_inner_appender_i();
}

// ============================================================================
// FileAppender::fill_inner_appender_i
// ============================================================================
void FileAppender::fill_inner_appender_i()
{
  try
  {
    std::string dev_key = "| " + m_dev_name + " |";
    std::deque<std::string> messages;
    yat::FileName fn_dst(m_file_name.path());
    std::vector<yat::String> names;

    if( fn_dst.path_exist() )
    {
      yat::FileEnum fe(m_file_name.path());
      while( fe.find() )
      {
        if( yat::String(fe.name_ext()).match(yat::String(m_file_name.name()).append("*")) )
          names.push_back(fe.full_name());
      }
      std::sort(names.begin(), names.end());
      if( names.size() > 1 )
      {
        if( names[0].match("*.log") )
        {
          // This file contains the most recent messages, we move its
          // name at the end of the list
          names.push_back(names[0]);
          names.erase(names.begin());
        }
      }

      std::size_t max_depth = InnerAppender::get_max_buffer_depth(m_dev);

      for( std::size_t i = 0; i < names.size(); ++i )
      {
        // Load log file content
        yat::File fi(names[names.size() - i - 1]);
        std::string content;
        fi.load(&content);

        std::string msg;
        if( !content.empty() )
        {
          // Browse content in reverse order and fill container
          std::size_t current_pos = content.size()-1, next_pos = 0;
          while( next_pos != std::string::npos && messages.size() < max_depth )
          {
            next_pos = content.rfind('\n', current_pos);
            if( next_pos != std::string::npos )
              msg = content.substr(next_pos+1, current_pos - next_pos);
            else
              msg = content.substr(0, current_pos);

            if( msg.find(dev_key) != std::string::npos )
              messages.insert(messages.begin(), msg);
            current_pos = next_pos - 1;
          }
        }
        if( messages.size() >= max_depth )
          // buffer full
          break;
      }
    }

    if( !messages.empty() )
    {
      InnerAppender::fill(m_dev, messages);
    }
  }
  catch(...)
  {
    // Silent catch
  }
}

// ============================================================================
// FileAppender::FileAppender
// ============================================================================
FileAppender::FileAppender()
  : log4tango::Appender("file-appender"), m_dev(0)
{
  //- noop
}

// ============================================================================
// FileAppender::~FileAppender
// ============================================================================
FileAppender::~FileAppender()
{
}

// ============================================================================
// FileAppender::initialize_i
// ============================================================================
void FileAppender::initialize_i(const Config& cfg)
{
  //- store host device
  m_dev = cfg.device_p;

  m_log_debug_level = cfg.log_debug_level;

  //- self register to device logger
  m_dev->get_logger()->add_appender(this);

  m_dev_name = m_dev->get_name();
  yat::String file_name = m_dev_name;
  file_name.replace('/', '_');

  Tango::Util* util = Tango::Util::instance();
  std::string ds_name, instance, ds_full_name = util->get_ds_name();
  yat::StringUtil::split(ds_full_name, '/', &ds_name, &instance);

  yat::StringTemplate tmpl;
  yat::EnvVariableInterpreter env;
  tmpl.add_symbol_interpreter(&env);
  std::string path = cfg.path;
  if( !tmpl.substitute(&path) )
  {
    throw yat::Exception("BAD_CONFIG",
                         std::string("Invalid log path: ").append(path).c_str(),
                         "FileAppender::initialize_i");
  }

  // set full name : base_path/ds_name/instance/instance.log
  m_file_name.set(path, file_name, "log");
  m_file_name.join(ds_name);
  m_file_name.join(instance);

  m_watcher_ptr.reset(new FileAppenderWatcher(m_file_name,
                                              cfg.max_file_days,
                                              cfg.max_file_size_kb,
                                              cfg.max_total_size_mb,
                                              cfg.min_available_mb,
                                              &FileAppender::m_rep_lock));

  // start task
  try
  {
    m_watcher_ptr->go();
  }
  catch(const yat::Exception &e)
  {
    m_watcher_ptr.reset();
    YAT_LOG_EXCEPTION(e);
  }
}

// ============================================================================
// FileAppender::close
// ============================================================================
void FileAppender::close()
{
  //- noop
}

// ============================================================================
// FileAppender::
// ============================================================================
int FileAppender::_append(const log4tango::LoggingEvent& event)
{
  //------------------------------------------------------------
  //- DO NOT LOG FROM THIS METHOD !!!
  //------------------------------------------------------------
  static const size_t max_time_str_len = 32;

  if( log4tango::Level::DEBUG != event.level || m_log_debug_level )
  {
    try
    {
      const std::string sep(" | ");
      //- reformat the incomming LoggingEvent
      //- date | LEVEL | [thr_id |] logger | message
      std::ostringstream oss;
    /* Should use the original event timestamp but due to millisec precision
       we have to use our own timestamp
    yat::Time tm_log;
    tm_log.set_double_unix(event.timestamp.get_seconds()
                           + (double(event.timestamp.get_microseconds()) / MICROSEC_PER_SEC));
    oss << tm_log.to_string(yat::Time::iso8601_z, yat::Time::microsec) << sep;
    */

      yat::String msg = event.message;
      msg.replace("\n", " - ");

      oss << yat::CurrentTime().to_string(yat::Time::iso8601_z, yat::Time::microsec) << sep;
      oss.width(5);
      oss << std::left << log4tango::Level::get_name(event.level) << sep;
      oss << std::hex << std::uppercase << event.thread_id << sep;
      oss << event.logger_name << sep << msg << '\n';

      //- push the message into the log file
      yat::File f(m_file_name);

      { yat::MutexLock guard(FileAppender::m_rep_lock);

        try
        {
          if( !f.file_exist() )
          {
            if( !f.path_exist() )
              f.mkdir();

            FileAppender::m_watcher_ptr->new_file();
          }
          f.append(oss.str());
        }
        catch(...)
        {
          return 0; // -1; because of crash at init_device
        }

        // minimal interval between two calls to check_file is limited
        YAT_FREQUENCY_LIMITED_STATEMENT( \
          m_watcher_ptr->check_file(),   \
          2); // seconds
      }
    }
    catch (...)
    {
      return 0; // -1; because of crash at init_device
    }
  }
  return 0;
}

// ============================================================================
// FileAppender::requires_layout
// ============================================================================
bool FileAppender::requires_layout () const
{
  return false;
}

// ============================================================================
// FileAppender::set_layout
// ============================================================================
void FileAppender::set_layout (log4tango::Layout* l)
{
  delete l;
}

} // namespace

