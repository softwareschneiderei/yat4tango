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
#include <yat/utils/Logging.h>
#include <yat4tango/PropertyHelper.h>
#include <yat/utils/String.h>
#include <yat/time/Time.h>
#include <yat4tango/InnerAppender.h>

namespace yat4tango
{

// ============================================================================
// InnerAppender static members
// ============================================================================
yat::Mutex InnerAppender::m_rep_lock;
InnerAppender::InnerAppenderRepository InnerAppender::m_rep;

const std::string PROPERTY_NAME = "Logging";
const std::size_t min_buffer_depth = 100;
const std::size_t max_buffer_depth = 100000;


// ============================================================================
// InnerAppender::Config::Config
// ============================================================================
InnerAppender::Config::Config(ColType c1, ColType c2, ColType c3, ColType c4,
                               ColType c5)
{
  if( default_multi == c1 )
  {
    // Default configuration in multi-columns mode
    c1 = date;
    c2 = level;
    c3 = message;
  }

  columns.push_back(c1);

  if( default_single != c1 )
  {
    if( no_def != c2 ) columns.push_back(c2);
    if( no_def != c3 ) columns.push_back(c3);
    if( no_def != c4 ) columns.push_back(c4);
    if( no_def != c5 ) columns.push_back(c5);
  }
  max_log_buffer_depth = INNER_APPENDER_DEFAULT_BUFFER_DEPTH;
  attribute_name = INNER_APPENDER_DEFAULT_ATTR_NAME;
  reverse_order = false;
  expert = false;
}

// ============================================================================
// InnerAppender::Config::set_columns
// ============================================================================
void InnerAppender::Config::set_columns(const yat::String& str)
{
  columns.clear();
  std::vector<yat::String> columns_def;
  str.split('|', &columns_def);
  yat::StringUtil::trim(&columns_def);

  if( columns_def.empty() )
    columns_def.push_back("default_single");

  for( std::size_t i = 0; i < columns_def.size(); ++i )
  {
    if( yat::StringUtil::is_equal_no_case(columns_def[i], "default_single") )
    {
      if( i > 0 )
        throw yat::Exception("BAD_CONFIG", "Bad columns spec", "InnerAppender::Config::set_columns");
      else
        columns.push_back(default_single);
      break;
    }
    if( yat::StringUtil::is_equal_no_case(columns_def[i], "default_multi") )
    {
      if( i > 0 )
        throw yat::Exception("BAD_CONFIG", "Bad columns spec", "InnerAppender::Config::set_columns");
      else
      {
        columns.push_back(date);
        columns.push_back(level);
        columns.push_back(message);
      }
      break;
    }
    if( yat::StringUtil::is_equal_no_case(columns_def[i], "date") )
      columns.push_back(date);
    else if( yat::StringUtil::is_equal_no_case(columns_def[i], "level") )
      columns.push_back(level);
    else if( yat::StringUtil::is_equal_no_case(columns_def[i], "message") )
      columns.push_back(message);
    else if( yat::StringUtil::is_equal_no_case(columns_def[i], "thread_id") )
      columns.push_back(thread_id);
    else if( yat::StringUtil::is_equal_no_case(columns_def[i], "logger_name") )
      columns.push_back(logger_name);
    else
      throw yat::Exception("BAD_CONFIG", "Bad columns spec", "InnerAppender::Config::set_columns");
  }
}

// ============================================================================
// InnerAppender::Config::set_order
// ============================================================================
void InnerAppender::Config::set_order(const std::string& v)
{
  if( yat::String(v).is_equal_no_case("backward") )
    reverse_order = true;
  else if( yat::String(v).is_equal_no_case("forward") )
    reverse_order = false;
  else
    throw yat::Exception("BAD_CONFIG", "Bad messages order specification for InnerAppender",
                         "InnerAppender::Config::set_order");
}

// ============================================================================
// InnerAppender::initialize
// ============================================================================
void InnerAppender::initialize(Tango::DeviceImpl * hd, std::size_t bd,
                                const std::string & an, Order o)
{
  Config cfg;
  cfg.max_log_buffer_depth = bd;
  cfg.attribute_name = an;
  cfg.reverse_order = asc == o ? false : true;
  initialize(hd, cfg);
}

// ============================================================================
// InnerAppender::initialize
// ============================================================================
void InnerAppender::initialize(Tango::DeviceImpl * hd, Order o)
{
  Config cfg;
  cfg.max_log_buffer_depth = INNER_APPENDER_DEFAULT_BUFFER_DEPTH;
  cfg.attribute_name = INNER_APPENDER_DEFAULT_ATTR_NAME;
  cfg.reverse_order = asc == o ? false : true;
  initialize(hd, cfg);
}

// ============================================================================
// InnerAppender::initialize
// ============================================================================
void InnerAppender::initialize(Tango::DeviceImpl * hd, const Config& cfg_,
                               bool no_prop)
{
  yat::MutexLock guard(InnerAppender::m_rep_lock);

  Config cfg = cfg_;

  if (! hd)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "unexpected null pointer to Tango device!",
                    "yat4tango::InnerAppender::initialize");
  }

  // minimal buffer depth
  if (min_buffer_depth > cfg.max_log_buffer_depth)
    cfg.max_log_buffer_depth = min_buffer_depth;
  if (max_buffer_depth < cfg.max_log_buffer_depth)
    cfg.max_log_buffer_depth = max_buffer_depth;

  //- do we already have an InnerAppender registered for the specified device?
  InnerAppenderIterator it = InnerAppender::m_rep.find(hd);
  if (it != InnerAppender::m_rep.end())
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "an InnerAppender is already associated to the specified device",
                    "InnerAppender::initialize");
  }

  InnerAppender * ia = new (std::nothrow) InnerAppender;
  if (! ia)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "yat4tango::InnerAppender instanciation failed!",
                    "yat4tango::InnerAppender::initialize");
  }

  ia->initialize_i(hd, cfg, no_prop);

  //- insert the InnerAppender into the local repository
  std::pair<InnerAppenderIterator, bool> insertion_result;
  insertion_result = InnerAppender::m_rep.insert(InnerAppenderEntry(hd, ia));
  if (insertion_result.second == false)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "failed to insert the InnerAppender into the local repository",
                    "yat4tango::InnerAppender::initialize");
  }
}

// ============================================================================
// InnerAppender::clear
// ============================================================================
void InnerAppender::clear(Tango::DeviceImpl * hd)
{
  if (! hd)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "unexpected null pointer to Tango device!",
                    "yat4tango::InnerAppender::clear");
  }

  yat::MutexLock guard(InnerAppender::m_rep_lock);

  //- do we already have an InnerAppender registered for the specified device?
  InnerAppenderIterator it = InnerAppender::m_rep.find(hd);
  if (it == InnerAppender::m_rep.end())
    return;

  it->second->clear_i();
}

// ============================================================================
// InnerAppender::release
// ============================================================================
void InnerAppender::release(Tango::DeviceImpl * hd)
{
  if (! hd)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "unexpected null pointer to Tango device!",
                    "yat4tango::InnerAppender::release");
  }

  yat::MutexLock guard(InnerAppender::m_rep_lock);

  //- do we have an InnerAppender registered for the specified device?
  InnerAppenderIterator it = InnerAppender::m_rep.find(hd);
  if (it == InnerAppender::m_rep.end())
    return;

  //- release the InnerAppender
  it->second->release_i();

  //- remove the InnerAppender from the local repository
  InnerAppender::m_rep.erase(it);
}

// ============================================================================
// InnerAppender::fill
// ============================================================================
void InnerAppender::fill(Tango::DeviceImpl* hd, std::deque<std::string>& messages)
{
  yat::MutexLock guard(InnerAppender::m_rep_lock);

  //- do we have an InnerAppender registered for the specified device?
  InnerAppenderIterator it = InnerAppender::m_rep.find(hd);
  if (it == InnerAppender::m_rep.end())
    return;

  //- fill the InnerAppender
  it->second->fill_i(messages);
}

// ============================================================================
// InnerAppender::get_max_buffer_depth
// ============================================================================
std::size_t InnerAppender::get_max_buffer_depth(Tango::DeviceImpl* hd)
{
  yat::MutexLock guard(InnerAppender::m_rep_lock);

  //- do we have an InnerAppender registered for the specified device?
  InnerAppenderIterator it = InnerAppender::m_rep.find(hd);
  if (it == InnerAppender::m_rep.end())
    THROW_DEVFAILED("DEVICE_ERROR",
                    "No InnerAppender for the device!",
                    "yat4tango::InnerAppender::get_buffer_depth");

  return it->second->m_cfg.max_log_buffer_depth;
}

// ============================================================================
// InnerAppender::InnerAppender
// ============================================================================
InnerAppender::InnerAppender()
  : log4tango::Appender ("inner-appender"),
    m_dev (0),
    m_dam (0),
    m_log_buffer_changed (true)
{
  //- noop
}

// ============================================================================
// InnerAppender::~InnerAppender
// ============================================================================
InnerAppender::~InnerAppender()
{
  DEBUG_ASSERT(m_dam == 0);
}

// ============================================================================
// InnerAppender::prop_to_config_i
// ============================================================================
void InnerAppender::prop_to_config_i(const std::vector<std::string>& prop_value)
{
  Config cfg = m_cfg;
  cfg.columns.clear();
  std::vector<yat::String> columns_def;

  for( std::size_t i = 0; i < prop_value.size(); ++i )
  {
    yat::String line = prop_value[i];
    line.trim();

    if( line.match("order*=*") )
    {
      yat::String v;
      line.extract_token_right('=', &v);
      v.trim();
      if( v.is_equal_no_case("backward") )
        cfg.reverse_order = true;
      else if( v.is_equal_no_case("forward") )
        cfg.reverse_order = false;
      // next line
      continue;
    }

    if( line.match("depth*=*") )
    {
      yat::String v;
      line.extract_token_right('=', &v);
      v.trim();
      std::size_t depth = v.to_num<std::size_t>();
      if( depth < min_buffer_depth )
        depth = min_buffer_depth;
      if( depth > max_buffer_depth )
        depth = max_buffer_depth;
      cfg.max_log_buffer_depth = depth;
      // next line
      continue;
    }

    while( !line.empty() )
    {
      std::size_t sep_pos = line.find_first_of(",;:.+-=/!*|");
      if( std::string::npos == sep_pos )
        sep_pos = line.find(' ');

      if( std::string::npos == sep_pos )
      {
        columns_def.push_back(line);
        break;
      }
      else
      {
        std::string v = line.substr(0, sep_pos);
        yat::StringUtil::trim(&v);
        if( !v.empty() )
          columns_def.push_back(v);
        line = line.substr(sep_pos + 1);
      }
    }
  }

  try
  {
    yat::String columns;
    columns.join(columns_def, '|');
    cfg.set_columns(columns);
    m_cfg = cfg;
  }
  catch(const yat::Exception& e)
  {
    YAT_LOG_EXCEPTION(e);
  }
}

// ============================================================================
// InnerAppender::initialize_i
// ============================================================================
void InnerAppender::initialize_i(Tango::DeviceImpl * hd, const Config& cfg, bool no_prop)
{
  //- check input
  if (! hd)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "unexpected null pointer to Tango device!",
                    "yat4tango::InnerAppender::init");
  }
  //- store host device
  m_dev = hd;

  std::vector<std::string> log_prop;
  try
  {
    log_prop = yat4tango::PropertyHelper::get_property< std::vector<std::string> >
                                               (hd, std::string(PROPERTY_NAME));
  }
  catch(...) {}

  m_cfg = cfg;
  if( !log_prop.empty() && !no_prop )
    prop_to_config_i(log_prop);

  //- instanciate the dynamic attribute manager
  m_dam = new (std::nothrow) DynamicAttributeManager(hd);
  if (! m_dam)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "yat4tango::DynamicAttributeManager instanciation failed!",
                    "yat4tango::InnerAppender::init");
  }

  //- describe the "log" dynamic attribute
  DynamicAttributeInfo dai;
  dai.dev = hd;
  dai.tai.name = m_cfg.attribute_name;
  if( 1 == m_cfg.columns.size() )
  {
    dai.tai.data_format = Tango::SPECTRUM;
    dai.tai.max_dim_x = m_cfg.max_log_buffer_depth;
  }
  else
  {
    dai.tai.data_format = Tango::IMAGE;
    dai.tai.max_dim_x = m_cfg.columns.size();
    dai.tai.max_dim_y = m_cfg.max_log_buffer_depth;
  }
  dai.tai.data_type = Tango::DEV_STRING;
  dai.tai.writable = Tango::READ;
  dai.tai.disp_level = Tango::OPERATOR;

  if( cfg.expert )
    dai.tai.disp_level = Tango::EXPERT;

  dai.rcb = DynamicAttributeReadCallback::instanciate(*this, &InnerAppender::read_callback);

  //- add it to the device interface
  m_dam->add_attribute(dai);

  //- self register to device logger
  m_dev->get_logger()->add_appender(this);
}

// ============================================================================
// InnerAppender::clear_i
// ============================================================================
void InnerAppender::clear_i()
{
  yat::MutexLock guard(m_lock);

  m_log_buffer.clear();
  m_log_buffer_changed = true;
}

// ============================================================================
// InnerAppender::fill_i
// ============================================================================
void InnerAppender::fill_i(std::deque<std::string>& messages)
{
  yat::MutexLock guard(m_lock);

  m_log_buffer.clear();
  std::size_t i = 0;
  if( messages.size() > m_cfg.max_log_buffer_depth )
    // Drop oldest messages to fit the requested buffer size
    i = messages.size() - m_cfg.max_log_buffer_depth;

  yat::String line;
  for(; i < messages.size(); ++i)
  {
    LogPtr ptr_log = new Log;
    line = messages[i];
    yat::String date, level, thr_id, logger;
    line.extract_token('|', &date);
    line.extract_token('|', &level);
    line.extract_token('|', &thr_id);
    line.extract_token('|', &logger);

    date.trim();
    level.trim();
    thr_id.trim();
    logger.trim();
    line.trim();

    // Try to read date/time
    yat::Time tm_log;
    try
    {
      tm_log.from_string(date, "%Y-%m-%dT%H:%M:%S.%6%Z");
      // suppress time zone bias
      date = tm_log.to_string(yat::Time::iso8601, yat::Time::microsec);
    }
    catch(...) {} // no matter, keep original date string

    ptr_log->date = date;
    ptr_log->level = level;
    ptr_log->thread_id = thr_id;
    ptr_log->logger_name = logger;
    ptr_log->message = line;

    std::ostringstream oss;
    oss << ptr_log->date << " [" << ptr_log->level << "] " << ptr_log->message;
    ptr_log->full_message = oss.str();

    m_log_buffer.push_back(ptr_log);
  }
  if( m_cfg.reverse_order )
    std::reverse(m_log_buffer.begin(), m_log_buffer.end());
}

// ============================================================================
// InnerAppender::release_i
// ============================================================================
void InnerAppender::release_i ()
{
  yat::MutexLock guard(m_lock);

  try
  {
    //- this will remove the "log" dynamic attribute from the device interface
    delete m_dam;
    m_dam = 0;
  }
  catch (...)
  {
    //- ignore error
  }

  //- self unregister to device logger (will generate a call to the destructor)
  m_dev->get_logger()->remove_appender(this);
}

// ============================================================================
// InnerAppender::close
// ============================================================================
void InnerAppender::close ()
{
  //- noop
}

// ============================================================================
// InnerAppender::read_callback (yat4tango::DynamicAttributeReadCallback impl)
// ============================================================================
void InnerAppender::read_callback (DynamicAttributeReadCallbackData& d)
{
  if (! d.tga)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "unexpected null pointer to Tango::Attribute [invalid yat4tango::DynamicAttributeReadCallbackData]",
                    "yat4tango::InnerAppender::read_callback");
  }

  {
    yat::MutexLock guard(m_lock);

    std::size_t n = m_cfg.columns.size();
    std::size_t l = m_log_buffer.size();

    if( m_log_buffer_changed )
    {
      m_log_array.length(l * n);

      std::string date, level, type, thread_id, logger_name, msg, line;
      for (size_t i = 0; i < l * n; i += n)
      {
        LogPtr& ptr_log = m_log_buffer[i/n];

        for (std::size_t c = 0; c < n; ++c)
        {
          switch( m_cfg.columns[c] )
          {
            case InnerAppender::default_single:
            {
              m_log_array[i + c] = ptr_log->full_message.c_str();
              break;
            }
            case InnerAppender::date:
              m_log_array[i + c] = ptr_log->date.c_str();
              break;
            case InnerAppender::level:
              m_log_array[i + c] = ptr_log->level.c_str();
              break;
            case InnerAppender::message:
              m_log_array[i + c] = ptr_log->message.c_str();
              break;
            case InnerAppender::thread_id:
              m_log_array[i + c] = ptr_log->thread_id.c_str();
              break;
            case InnerAppender::logger_name:
              m_log_array[i + c] = ptr_log->logger_name.c_str();
              break;
            default:
              // Oups!!
              m_log_array[i + c] = "!!!bad log format!!!";
              break;
          }
        }
      }
      m_log_buffer_changed = false;
    }

    if( n > 1 )
      d.tga->set_value(m_log_array.get_buffer(), n, l);
    else
      d.tga->set_value(m_log_array.get_buffer(), l);

    // Purge
    m_log_trash_buffer.clear();
  }
}

// ============================================================================
// InnerAppender::
// ============================================================================
int InnerAppender::_append (const log4tango::LoggingEvent& event)
{
  //------------------------------------------------------------
  //- DO NOT LOG FROM THIS METHOD !!!
  //------------------------------------------------------------

  static const std::size_t max_time_str_len = 32;

  try
  {
    //- reformat the incomming LoggingEvent
    LogPtr ptr_log = new Log;

    /* Should use the original event timestamp but due to millisec precision
       we have to use our own timestamp
    yat::Time tm_log;
    tm_log.set_double_unix(event.timestamp.get_seconds()
                           + (double(event.timestamp.get_microseconds()) / MICROSEC_PER_SEC));
    ptr_log->date = tm_log.to_string(yat::Time::iso8601, yat::Time::microsec);
    */

    ptr_log->date = yat::CurrentTime().to_string(yat::Time::iso8601, yat::Time::microsec);
    ptr_log->level = log4tango::Level::get_name(event.level);
    { std::ostringstream oss;
      oss << std::hex << std::uppercase << event.thread_id;
      ptr_log->thread_id = oss.str();
    }
    ptr_log->logger_name = event.logger_name;
    ptr_log->message = event.message;
    yat::StringUtil::replace(&ptr_log->message, "\n", " - ");

    { std::ostringstream oss;
      oss << ptr_log->date << " [" << ptr_log->level << "] ";
      for( yat::uint32 i = 0; i < 5 - ptr_log->level.size(); ++i)
        oss << ' ';
      oss << ptr_log->message;
      ptr_log->full_message = oss.str();
    }
    LogPtr ptr_to_be_deleted_log;

    //- push the log into the log buffer
    {
      yat::MutexLock guard(m_lock);

      //- push
      if( m_cfg.reverse_order )
      {
        m_log_buffer.insert(m_log_buffer.begin(), ptr_log);
        //- pop oldest log if necessary
        if (m_log_buffer.size() > m_cfg.max_log_buffer_depth)
        {
          ptr_to_be_deleted_log = m_log_buffer.back();
          m_log_buffer.pop_back();
        }
      }
      else
      {
        m_log_buffer.push_back(ptr_log);
        //- pop oldest log if necessary
        if (m_log_buffer.size() > m_cfg.max_log_buffer_depth)
        {
          ptr_to_be_deleted_log = m_log_buffer.front();
          m_log_buffer.pop_front();
        }
      }

      if( ptr_to_be_deleted_log &&
          m_log_trash_buffer.size() < m_cfg.max_log_buffer_depth )
      {
        // If the trash buffer got size > max buffer depth the newest messages
        // it contain never have been referenced by m_log_array
        m_log_trash_buffer.push_back(ptr_to_be_deleted_log);
      }
      //- mark buffer content as changed
      m_log_buffer_changed = true;
    }
  }
  catch (...)
  {
    return -1;
  }
  return 0;
}

// ============================================================================
// InnerAppender::requires_layout
// ============================================================================
bool InnerAppender::requires_layout () const
{
  return false;
}

// ============================================================================
// InnerAppender::set_layout
// ============================================================================
void InnerAppender::set_layout (log4tango::Layout* l)
{
  delete l;
}

} // namespace

