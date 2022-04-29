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
// Copyright (c) 2004-2016 The Tango Community
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
#include <yat/utils/StringTokenizer.h>
#include <yat4tango/DynamicAttributeManager.h>

namespace yat4tango
{

// ============================================================================
// DynamicAttributeManager::ctor
// ============================================================================
DynamicAttributeManager::DynamicAttributeManager(Tango::DeviceImpl * _dev)
  : Tango::LogAdapter(_dev), dev_(_dev)
{
  YAT4TANGO_TRACE("DynamicAttributeManager::DynamicAttributeManager");
}

// ============================================================================
// DynamicAttributeManager::dtor
// ============================================================================
DynamicAttributeManager::~DynamicAttributeManager()
{
  YAT4TANGO_TRACE("DynamicAttributeManager::~DynamicAttributeManager");

  this->remove_attributes();
}

// ============================================================================
// DynamicAttributeManager::set_host_device
// ============================================================================
void DynamicAttributeManager::set_host_device (Tango::DeviceImpl * h)
{
  if ( this->dev_ && h != this->dev_ )
  {
    THROW_DEVFAILED( "CONFIGURATION_ERROR",
                     "can't change the Tango device associated with a DynamicAttributeManager [already linked to a device]",
                     "DynamicAttributeManager::add_attribute");
  }

  if ( ! h )
  {
    THROW_DEVFAILED( "INVALID_ARG",
                     "unvalid Tango::DeviceImpl specified [unexcpected null pointer]",
                     "DynamicCommandManager::add_attribute");
  }

  this->dev_ = h;
}

// ============================================================================
// DynamicAttributeManager::add_attributes
// ============================================================================
void DynamicAttributeManager::add_attributes (const std::vector<DynamicAttributeInfo>& ai)
{
  YAT4TANGO_TRACE("DynamicAttributeManager::add_attributes");

  for (size_t i = 0; i < ai.size(); i++)
    this->add_attribute(ai[i]);
}

// ============================================================================
// DynamicAttributeManager::add_attributes
// ============================================================================
void DynamicAttributeManager::add_attributes (const std::vector<ForwardedAttributeInfo>& ai)
{
  YAT4TANGO_TRACE("DynamicAttributeManager::add_attributes");

  for (size_t i = 0; i < ai.size(); i++)
    this->add_attribute(ai[i]);
}

// ============================================================================
// DynamicAttributeManager::add_attributes
// ============================================================================
void DynamicAttributeManager::add_attributes (const std::vector<std::string>& urls)
{
  YAT4TANGO_TRACE("DynamicAttributeManager::add_attributes");

  for (size_t i = 0; i < urls.size(); i++)
    this->add_attribute(urls[i]);
}

// ============================================================================
// DynamicAttributeManager::add_attribute
// ============================================================================
void DynamicAttributeManager::add_attribute (const yat4tango::DynamicAttributeInfo& dai)
{
  YAT4TANGO_TRACE("DynamicAttributeManager::add_attribute[const yat4tango::DynamicAttributeInfo&]");

  if ( ! this->dev_ )
  {
    THROW_DEVFAILED( "INITIALIZATION_ERROR",
                     "the DynamicAttributeManager is not properly initialized [no associated device]",
                     "DynamicAttributeManager::add_attribute");
  }

  //- adapt some DynamicAttributeInfo members
  DynamicAttributeInfo & rw_dai = const_cast<DynamicAttributeInfo&>(dai);
  //- force host device to <this->dev_>
  rw_dai.dev = this->dev_;
  //- if no local attribute name specify then use original name
  if ( rw_dai.lan == yat4tango::KeepOriginalAttributeName )
    rw_dai.lan = rw_dai.tai.name;

  //- check attribute does not already exist
  DynamicAttributeIterator it = this->rep_.find(dai.lan);
  if (it != this->rep_.end())
  {
    std::ostringstream oss;
    oss << "couldn't add dynamic attribute <"
        << dai.lan
        << "> - another dynamic attribute already exists with same name";
    THROW_DEVFAILED("OPERATION_NOT_ALLOWED",
                    oss.str().c_str(),
                    "DynamicAttributeManager::add_attribute");
  }

  //- instanciate the dynamic attribute (might throw an exception)
  DynamicAttribute * da = DynamicAttributeFactory::instance().create_attribute(dai);

  //- add it to both the device and the repository
  try
  {
    this->add_attribute(da);
  }
  catch (Tango::DevFailed&)
  {
    delete da;
    throw;
  }
  catch (...)
  {
    delete da;
    THROW_DEVFAILED( "UNKNOWN_ERROR",
                     "unknown exception caught while trying to add the dynamic attribute to the device interface",
                     "DynamicAttributeManager::add_attribute");
  }
}

// ============================================================================
// DynamicAttributeManager::add_attribute
// ============================================================================
void DynamicAttributeManager::add_attribute (yat4tango::DynamicAttribute * da)
{
  YAT4TANGO_TRACE("DynamicAttributeManager::add_attribute[DynamicAttribute*]");

  if ( ! this->dev_)
  {
    THROW_DEVFAILED( "INITIALIZATION_ERROR",
                     "the DynamicAttributeManager is not properly initialized [no associated device]",
                     "DynamicAttributeManager::add_attribute");
  }

  //- be sure the DynamicAttribute is valid
  if (! da || ! da->get_tango_attribute()) return;

  //- get the dyn. attr. name
  std::string & local_attr_name = da->dai_.lan;

  //- be sure 'local' attribute name is properly specified
  if ( local_attr_name == yat4tango::KeepOriginalAttributeName )
    local_attr_name = da->get_attribute_info().name;

  //- check attribute does not already exist
  DynamicAttributeIterator it = this->rep_.find(local_attr_name);
  if (it != this->rep_.end())
  {
    std::ostringstream oss;
    oss << "couldn't add dynamic attribute <"
        << local_attr_name
        << "> - another dynamic attribute already exists with same name";
    THROW_DEVFAILED("OPERATION_NOT_ALLOWED",
                    oss.str().c_str(),
                    "DynamicAttributeManager::add_attribute");
  }

  //- add the dynamic attribute to the device interface
  try
  {
    this->dev_->add_attribute(da->get_tango_attribute());
  }
  catch (Tango::DevFailed& ex)
  {
    std::ostringstream oss;
    oss << "couldn't add dynamic attribute <"
        << local_attr_name
        << "> - Tango exception caught";
    RETHROW_DEVFAILED(ex,
                      "API_ERROR",
                      oss.str().c_str(),
                      "DynamicAttributeManager::add_attribute");
  }
  catch (...)
  {
    std::ostringstream oss;
    oss << "couldn't add dynamic attribute <"
        << local_attr_name
        << "> - Tango exception caught";
    THROW_DEVFAILED("API_ERROR",
                    oss.str().c_str(),
                    "DynamicAttributeManager::add_attribute");
  }

  //- insert the attribute into the local repository
  std::pair<DynamicAttributeIterator, bool> insertion_result;
  insertion_result = this->rep_.insert(DynamicAttributeEntry(local_attr_name, da));
  if (insertion_result.second == false)
  {
    THROW_DEVFAILED("INTERNAL_ERROR",
                    "failed to insert the dynamic attribute into the local repository",
                    "DynamicAttributeManager::add_attribute");
  }
}

// ============================================================================
// DynamicAttributeManager::add_attribute
// ============================================================================
void DynamicAttributeManager::add_attribute (const std::string& url, const std::string& lan, bool rdo)
{
  YAT4TANGO_TRACE("DynamicAttributeManager::add_attribute");

  if ( ! this->dev_ )
  {
    THROW_DEVFAILED( "INITIALIZATION_ERROR",
                     "the DynamicAttributeManager is not properly initialized [no associated device]",
                     "DynamicAttributeManager::add_attribute");
  }

  //- extract dev name & attr name from url
  yat::StringTokenizer st(url, "/");
  size_t nt = st.count_tokens();
  if ( nt > 1 && ( nt < 4 || nt > 5 ) )
  {
    std::ostringstream oss;
    oss << "invalid fully qualified attribute name <"
        << url
        << "> - should be something like [db-host:db-port/]tango/dev/name/attr-name";
    THROW_DEVFAILED("API_ERROR",
                    oss.str().c_str(),
                    "DynamicAttributeManager::add_attribute");
  }

  //- dev name
  std::string dev;
  //- 'remote' attr name
  std::string remote_attr_name;

  if ( nt > 1 )
  {
    //- fully qualified attribute name
    if ( nt == 5 )
      st.next_token();
    st.next_token();
    st.next_token();
    st.next_token();
    remote_attr_name = st.next_token();
  }
  else
  {
    //- alias ?
    try
    {
      Tango::AttributeProxy ap(url.c_str());
      remote_attr_name = ap.name();
    }
    catch (Tango::DevFailed& e)
    {
      std::ostringstream oss;
      oss << "invalid/unknown attribute alias - '"
          << url
          << "' is invalid";
      RETHROW_DEVFAILED(e,
                        "API_ERROR",
                        oss.str().c_str(),
                        "DynamicAttributeFactory::add_attribute");
    }
    catch (...)
    {
      std::ostringstream oss;
      oss << "unknown exception caught while trying to forward attribute '"
          << url
          << "'";
      THROW_DEVFAILED("API_ERROR",
                      oss.str().c_str(),
                      "DynamicAttributeFactory::add_attribute");
    }
  }

  //- forwarded attribute name (i.e. local attribute name)
  std::string fwd_as;
  if ( lan != yat4tango::KeepOriginalAttributeName )
    fwd_as = lan;
  else
    fwd_as = remote_attr_name;

  //- check attribute does not already exist
  DynamicAttributeIterator it = this->rep_.find(fwd_as);
  if (it != this->rep_.end())
  {
    std::ostringstream oss;
    oss << "couldn't add dynamic attribute <"
        << fwd_as
        << "> - another dynamic attribute already exists with same name";
    THROW_DEVFAILED("OPERATION_NOT_ALLOWED",
                    oss.str().c_str(),
                    "DynamicAttributeManager::add_attribute");
  }

  //- instanciate the dynamic attribute (might throw an exception)
  yat4tango::ForwardedAttributeInfo ai;
  ai.dev = this->dev_;
  ai.url = url;
  ai.lan = fwd_as;
  ai.rdo = rdo;
  DynamicAttribute * da = DynamicAttributeFactory::instance().create_attribute(ai);

  //- add it to both the device and the repository
  try
  {
    this->add_attribute(da);
  }
  catch (Tango::DevFailed&)
  {
    delete da;
    throw;
  }
  catch (...)
  {
    delete da;
    THROW_DEVFAILED( "UNKNOWN_ERROR",
                     "unknown exception caught while trying to add the dynamic attribute to the device interface",
                     "DynamicAttributeManager::add_attribute");
  }
}

// ============================================================================
// DynamicAttributeManager::add_attribute
// ============================================================================
void DynamicAttributeManager::add_attribute (const yat4tango::ForwardedAttributeInfo& dai)
{
  YAT4TANGO_TRACE("DynamicAttributeManager::add_attribute[yat4tango::ForwardedAttributeInfo&]");

  this->add_attribute(dai.url, dai.lan);
}

// ============================================================================
// DynamicAttributeManager::remove_attribute
// ============================================================================
void DynamicAttributeManager::remove_attribute (const std::string& an)
{
  YAT4TANGO_TRACE("DynamicAttributeManager::remove_attribute");

  if ( ! this->dev_ )
    return;

  //- attribute exists?
  DynamicAttributeIterator it = this->rep_.find(an);
  if (it == this->rep_.end())
  {
    THROW_DEVFAILED("ATTRIBUTE_NOT_FOUND",
                    "no such dynamic attribute [unknown attribute name]",
                    "DynamicAttributeManager::add_attribute");
  }
  //- found... remove it from the device
  try
  {
    DynamicAttribute * da = (*it).second;
#if (TANGO_VERSION_MAJOR >= 8)
    bool cdb = da->dai_.memorized ? false : da->dai_.cdb;
    this->dev_->remove_attribute(da->get_tango_attribute(), false, cdb);
#else
    this->dev_->remove_attribute(da->get_tango_attribute(), false);
#endif
    delete da;
  }
  catch (Tango::DevFailed& ex)
  {
    RETHROW_DEVFAILED(ex,
                      "INTERNAL_ERROR",
                      "failed to remove dynamic attribute from the device interface",
                      "DynamicAttributeManager::remove_attribute");
  }
  catch (...)
  {
    THROW_DEVFAILED("UNKNOWN_ERROR",
                    "failed to remove dynamic attribute from the device interface",
                    "DynamicAttributeManager::remove_attribute");
  }

  //- then... remove is from the local repository
  this->rep_.erase(it);
};

// ============================================================================
// DynamicAttributeManager::remove_attributes
// ============================================================================
void DynamicAttributeManager::remove_attributes()
{
  YAT4TANGO_TRACE("DynamicAttributeManager::remove_attributes");

  if ( ! this->dev_ )
    return;

  for (DynamicAttributeIterator it = this->rep_.begin(); it != this->rep_.end(); ++it)
  {
    //- remove attribute from the device interface
    try
    {
      DynamicAttribute * da = (*it).second;
#if (TANGO_VERSION_MAJOR >= 8)
      bool cdb = da->dai_.memorized ? false : da->dai_.cdb;
      this->dev_->remove_attribute(da->get_tango_attribute(), false, cdb);
#else
      this->dev_->remove_attribute(da->get_tango_attribute(), false);
#endif
      delete da;
    }
    catch (Tango::DevFailed& ex)
    {
      RETHROW_DEVFAILED(ex,
                        "INTERNAL_ERROR",
                        "failed to remove dynamic attribute from the device interface",
                        "DynamicAttributeManager::remove_attribute");
    }
    catch (...)
    {
      THROW_DEVFAILED("UNKNOWN_ERROR",
                      "failed to remove dynamic attribute from the device interface",
                      "DynamicAttributeManager::remove_attribute");
    }
  }

  //- finally, clear the repository...
  this->rep_.clear();
}

} // namespace


