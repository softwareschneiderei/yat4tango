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
#include <yat/file/FileName.h>
#include <yat4tango/PropertyHelper.h>
#include <yat4tango/InnerAppender.h>
#include <yat4tango/FileAppender.h>
#include <yat4tango/YatLogAdapter.h>
#include <yat4tango/Logging.h>

namespace yat4tango
{

const std::string ds_attr = "attr";
const std::string ds_order = "order";
const std::string ds_depth = "depth";
const std::string ds_layout = "layout";
const std::string ds_expert = "expert";

const std::string fs_enable = "enable";
const std::string fs_path = "path";
const std::string fs_max_file_size = "max_file_size_kb";
const std::string fs_max_file_days = "max_file_days";
const std::string fs_max_total_size = "max_total_size_mb";
const std::string fs_min_available_mb = "min_available_mb";
const std::string fs_log_debug_lvl = "log_debug_level";

// ============================================================================
// init_innerappender
// ============================================================================
void init_innerappender(Tango::DeviceImpl* device_p, const yat::CfgFile& cfg_f, const yat::StringDictionary& def)
{
  // Read default values
  std::string def_columns, def_order;
  InnerAppender::Config def_cfg;
  def_cfg.attribute_name = def.get(ds_attr).value_or(INNER_APPENDER_DEFAULT_ATTR_NAME);
  def_cfg.max_log_buffer_depth = def.get_numeric<yat::uint32>(ds_depth).value_or(INNER_APPENDER_DEFAULT_BUFFER_DEPTH);
  def_columns = def.get(ds_layout).value_or(INNER_APPENDER_DEFAULT_LAYOUT);
  def_order = def.get(ds_order).value_or(INNER_APPENDER_DEFAULT_ORDER);
  def_cfg.expert = def.get_bool(ds_expert).value_or(INNER_APPENDER_DEFAULT_EXPERT_FLAG);

  InnerAppender::Config cfg = def_cfg;
  std::string columns = def_columns, order = def_order;
  try
  {
    cfg_f.set_section("device_stream");
    yat::StringDictionary kv(cfg_f.get_parameters());
    cfg.attribute_name = kv.get(ds_attr).value_or(def_cfg.attribute_name);
    cfg.max_log_buffer_depth = kv.get_numeric<yat::uint32>(ds_depth).value_or(def_cfg.max_log_buffer_depth);
    columns = kv.get(ds_layout).value_or(def_columns);
    order = kv.get(ds_order).value_or(def_order);
    cfg.expert = kv.get_bool(ds_expert).value_or(def_cfg.expert);
  }
  catch(...) {} // keep default values

  try
  {
    cfg.set_columns(columns);
  }
  catch(const yat::Exception& e)
  {
    cfg.set_columns(INNER_APPENDER_DEFAULT_LAYOUT);
  }
  try
  {
    cfg.set_order(order);
  }
  catch(const yat::Exception& e)
  {
    cfg.set_order(INNER_APPENDER_DEFAULT_ORDER);
  }

  InnerAppender::initialize(device_p, cfg, true);
}

// ============================================================================
// init_fileappender
// ============================================================================
void init_fileappender(Tango::DeviceImpl* device_p, const yat::CfgFile& cfg_f, const yat::StringDictionary& def)
{
  FileAppender::Config def_cfg;

  // Read default values
  bool def_enabled = def.get_bool(fs_enable).value_or(false);
  def_cfg.path = def.get(fs_path).value_or(FILE_APPENDER_DEFAULT_PATH);
  def_cfg.max_file_days = def.get_numeric<yat::uint32>(fs_max_file_days).value_or(FILE_APPENDER_DEFAULT_MAX_FILE_DAYS);
  def_cfg.max_file_size_kb = def.get_numeric<yat::uint32>(fs_max_file_size).value_or(FILE_APPENDER_DEFAULT_MAX_FILE_SIZE);
  def_cfg.max_total_size_mb = def.get_numeric<yat::uint32>(fs_max_total_size).value_or(FILE_APPENDER_DEFAULT_MAX_ABSOLUTE_SIZE);
  def_cfg.min_available_mb = def.get_numeric<yat::uint32>(fs_min_available_mb).value_or(FILE_APPENDER_DEFAULT_FS_MIN_AVAILABLE_MB);
  def_cfg.log_debug_level = def.get_bool(fs_log_debug_lvl).value_or(FILE_APPENDER_DEFAULT_DEBUG_LVL);

  FileAppender::Config cfg = def_cfg;
  bool enabled = def_enabled;
  cfg.device_p = device_p;
  try
  {
    cfg_f.set_section("file_stream");
    yat::StringDictionary kv(cfg_f.get_parameters());
    enabled = kv.get_bool(fs_enable).value_or(def_enabled);
    cfg.path = kv.get(fs_path).value_or(def_cfg.path);
    cfg.max_file_days = kv.get_numeric<yat::uint32>(fs_max_file_days).value_or(def_cfg.max_file_days);
    cfg.max_file_size_kb = kv.get_numeric<yat::uint32>(fs_max_file_size).value_or(def_cfg.max_file_size_kb);
    cfg.max_total_size_mb = kv.get_numeric<yat::uint32>(fs_max_total_size).value_or(def_cfg.max_total_size_mb);
    cfg.min_available_mb = kv.get_numeric<yat::uint32>(fs_min_available_mb).value_or(def_cfg.min_available_mb);
    cfg.log_debug_level = kv.get_bool(fs_log_debug_lvl).value_or(def_cfg.log_debug_level);
  }
  catch(...) {}

  if( enabled )
    FileAppender::initialize(cfg);
}

// ============================================================================
// Logging::initialize
// ============================================================================
void Logging::initialize(Tango::DeviceImpl* hd)
{
  initialize(hd, "Logging");
}

// ============================================================================
// Logging::initialize
// ============================================================================
void Logging::initialize(Tango::DeviceImpl* hd, const std::string& prop_name)
{
  Tango::Database db;

  // Read default values from free property 'Logging'

  yat::StringDictionary ia_kv;
  try
  {
    yat::CfgFile cfg_f;
    std::vector<std::string> raw_content;
    Tango::DbData db_data;
    db_data.push_back(Tango::DbDatum("DeviceStream"));
    db.get_property("Logging", db_data);
    db_data[0] >> raw_content;
    cfg_f.load_from_string(yat::StringUtil::join(raw_content, '\n'));
    cfg_f.set_section(CFG_FILE_DEFAULT_SECTION);
    ia_kv = cfg_f.get_parameters();
  }
  catch(...) {}

  yat::StringDictionary fa_kv;
  try
  {
    yat::CfgFile cfg_f;
    std::vector<std::string> raw_content;
    Tango::DbData db_data;
    db_data.push_back(Tango::DbDatum("FileStream"));
    db.get_property("Logging", db_data);
    db_data[0] >> raw_content;
    cfg_f.load_from_string(yat::StringUtil::join(raw_content, '\n'));
    cfg_f.set_section(CFG_FILE_DEFAULT_SECTION);
    fa_kv = cfg_f.get_parameters();
  }
  catch(...) {}


  yat::CfgFile cfg_f;
  try
  {
    std::vector<std::string> log_prop;
    log_prop = yat4tango::PropertyHelper::get_property< std::vector<std::string> >(hd, prop_name);
    yat::String prop;
    prop.join(log_prop, '\n');
    cfg_f.load_from_string(prop);
  }
  catch(...) {}

  init_innerappender(hd, cfg_f, ia_kv);
  init_fileappender(hd, cfg_f, fa_kv);

	// YAT logs to Tango
  yat4tango::YatLogAdapter::initialize(hd);
}

// ============================================================================
// Logging::release
// ============================================================================
void Logging::release(Tango::DeviceImpl* hd)
{
  yat4tango::FileAppender::release(hd);
  yat4tango::InnerAppender::release(hd);
  yat4tango::YatLogAdapter::release();
}

// ============================================================================
// Logging::archive_and_reset
// ============================================================================
void Logging::archive_and_reset(Tango::DeviceImpl* device_p)
{
  if( FileAppender::is_initialized() )
  {
    FileAppender::archive(device_p);
  }
}

// ============================================================================
// Logging::clear_log
// ============================================================================
void Logging::clear_log(Tango::DeviceImpl* device_p)
{
  InnerAppender::clear(device_p);
}

} // namespace


