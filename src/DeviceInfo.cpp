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
#include <yat4tango/DeviceInfo.h>

namespace yat4tango
{
// ============================================================================
// DeviceInfo static members
// ============================================================================
yat::Mutex DeviceInfo::s_rep_lock;
DeviceInfo::DeviceInfoRepository DeviceInfo::s_rep;

// ============================================================================
// DeviceInfo::instanciate
// ============================================================================
void DeviceInfo::initialize (Tango::DeviceImpl* hd,
                             const std::string& project_name,
                             const std::string& project_version,
                             const std::string& attribute_name)
{
  yat::MutexLock guard(DeviceInfo::s_rep_lock);

  if (! hd)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "unexpected null pointer to Tango device!",
                    "yat4tango::DeviceInfo::initialize");
  }

  //- do we already have an DeviceInfo registered for the specified device?
  DeviceInfoIterator it = DeviceInfo::s_rep.find(hd);
  if (it != DeviceInfo::s_rep.end())
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "a DeviceInfo is already associated to the specified device",
                    "DeviceInfo::initialize");
  }

  DeviceInfo * ia = new (std::nothrow) DeviceInfo;
  if (! ia)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "yat4tango::DeviceInfo instanciation failed!",
                    "yat4tango::DeviceInfo::initialize");
  }

  ia->initialize_i(hd, project_name, project_version, attribute_name);

  //- insert the DeviceInfo into the local repository
  std::pair<DeviceInfoIterator, bool> insertion_result;
  insertion_result = DeviceInfo::s_rep.insert(DeviceInfoEntry(hd, ia));
  if (insertion_result.second == false)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "failed to insert the DeviceInfo into the local repository",
                    "yat4tango::DeviceInfo::initialize");
  }

}

// ============================================================================
// DeviceInfo::add_dependency
// ============================================================================
void DeviceInfo::add_dependency(Tango::DeviceImpl* hd, const std::string& name, const std::string& version)
{
  yat::MutexLock guard(DeviceInfo::s_rep_lock);
  DeviceInfoIterator it = DeviceInfo::s_rep.find(hd);
  if (it == DeviceInfo::s_rep.end())
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "No DeviceInfo associated to the specified device",
                    "DeviceInfo::add_dependency_version");
  }

  it->second->add_dependency_i(name, version);
}

// ============================================================================
// DeviceInfo::release
// ============================================================================
void DeviceInfo::release (Tango::DeviceImpl * hd)
{
  yat::MutexLock guard(DeviceInfo::s_rep_lock);

  if (!hd)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "unexpected null pointer to Tango device!",
                    "yat4tango::DeviceInfo::release");
  }

  //- do we have an DeviceInfo registered for the specified device?
  DeviceInfoIterator it = DeviceInfo::s_rep.find(hd);
  if (it == DeviceInfo::s_rep.end())
    return;

  //- release the DeviceInfo
  it->second->release_i();

  //- remove the DeviceInfo from the local repository
  DeviceInfo::s_rep.erase(it);
}

// ============================================================================
// DeviceInfo::DeviceInfo
// ============================================================================
DeviceInfo::DeviceInfo () :
    m_dev (0),
    m_dam (0)
{
  //- noop
}

// ============================================================================
// DeviceInfo::~DeviceInfo
// ============================================================================
DeviceInfo::~DeviceInfo ()
{
  DEBUG_ASSERT(m_dam == 0);
}

// ============================================================================
// DeviceInfo::initialize_i
// ============================================================================
void DeviceInfo::initialize_i (Tango::DeviceImpl* hd,
                               const std::string& project_name,
                               const std::string& project_version,
                               const std::string& attr_name)
{
  //- store host device
  m_dev = hd;

  //- instanciate the dynamic attribute manager
  m_dam = new (std::nothrow) DynamicAttributeManager(hd);
  if (! m_dam)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "yat4tango::DynamicAttributeManager instanciation failed!",
                    "yat4tango::DeviceInfo::init");
  }

  //- describe the "log" dynamic attribute
  DynamicAttributeInfo dai;
  dai.dev = hd;
  dai.tai.name = attr_name;
  dai.tai.data_format = Tango::SPECTRUM;
  dai.tai.data_type = Tango::DEV_STRING;
  dai.tai.writable = Tango::READ;
  dai.tai.disp_level = Tango::EXPERT;
  dai.tai.max_dim_x = 1024;
  dai.rcb = DynamicAttributeReadCallback::instanciate(*this, &DeviceInfo::read_callback);

  //- add it to the device interface
  m_dam->add_attribute(dai);

  //- Project version. YAT version is automatically included
  yat::Version::set(project_name, project_version);

  //- This library
  yat::Version::add_dependency("YAT4TANGO", YAT_XSTR(YAT4TANGO_PROJECT_VERSION));

  //- Tango lib version
  std::ostringstream oss;
  oss << std::fixed << Tango::Util::instance()->get_tango_lib_release();
  yat::String s = oss.str();
  //### Pb with VC12
  //s.from_num<long>(Tango::Util::instance()->get_tango_lib_release());
  if( s.size() == 3 )
  {
    std::ostringstream oss1;
    oss1 << s[0] << '.' << s[1] << '.' << s[2];
    yat::Version::add_dependency("TANGO", oss1.str());
    std::ostringstream oss2;
    oss2 << hd->get_dev_idl_version();
    yat::Version::add_dependency_module("Server IDL", oss2.str() );
  }

  yat::StringUtil::split(yat::Version::get(), '\n', &m_buffer, true);
  m_buffer_changed = true;
}

// ============================================================================
// DeviceInfo::add_dependency_i
// ============================================================================
void DeviceInfo::add_dependency_i(const std::string& name, const std::string& version)
{
  yat::Version::add_dependency(name, version);
  yat::StringUtil::split(yat::Version::get(), '\n', &m_buffer, true);
  m_buffer_changed = true;
}


// ============================================================================
// DeviceInfo::release_i
// ============================================================================
void DeviceInfo::release_i ()
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
}

// ============================================================================
// DeviceInfo::close
// ============================================================================
void DeviceInfo::close ()
{
  //- noop
}

// ============================================================================
// DeviceInfo::read_callback (yat4tango::DynamicAttributeReadCallback impl)
// ============================================================================
void DeviceInfo::read_callback (DynamicAttributeReadCallbackData& d)
{
  static Tango::DevVarStringArray __log_array__;

  if (! d.tga)
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "unexpected null pointer to Tango::Attribute [invalid yat4tango::DynamicAttributeReadCallbackData]",
                    "yat4tango::DeviceInfo::read_callback");
  }

  {
    yat::MutexLock guard(m_lock);

    if (m_buffer_changed)
    {
      size_t l = m_buffer.size();

      __log_array__.length(l);

      for (size_t i = 0; i < l; i++)
        __log_array__[i] = m_buffer[i].c_str();

      m_buffer_changed = false;
    }

    d.tga->set_value(__log_array__.get_buffer(), __log_array__.length());
  }
}

} // namespace

