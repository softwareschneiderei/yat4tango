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
#include <yat4tango/AttributeGroup.h>

namespace yat4tango
{

//=============================================================================
// STATIC MEMBERS
//=============================================================================
bool AttributeGroupReply::exception_enabled = false;

// ============================================================================
// AttributeGroupReply::AttributeGroupReply
// ============================================================================
AttributeGroupReply::AttributeGroupReply ()
  : dev_name_m("unknown"),
    attr_name_m("unknown"),
    has_failed_m(false)
{
  //- noop
}

// ============================================================================
// AttributeGroupReply::AttributeGroupReply
// ============================================================================
AttributeGroupReply::AttributeGroupReply (const AttributeGroupReply& src)
{
  *this = src;
}

// ============================================================================
// AttributeGroupReply::AttributeGroupReply
// ============================================================================
AttributeGroupReply::AttributeGroupReply (const std::string& _dev_name,
                                          const std::string& _attr_name)
 : dev_name_m(_dev_name),
   attr_name_m(_attr_name),
   has_failed_m(false)
{
  //- noop
}

// ============================================================================
// AttributeGroupReply::AttributeGroupReply
// ============================================================================
AttributeGroupReply::AttributeGroupReply (const std::string& _dev_name,
                                          const std::string& _attr_name,
                                          const Tango::DevFailed& exception)
 : dev_name_m(_dev_name),
   attr_name_m(_attr_name),
   has_failed_m(true),
   exception_m(exception)

{
  //- noop
}

// ============================================================================
// AttributeGroupReply::~AttributeGroupReply
// ============================================================================
AttributeGroupReply::~AttributeGroupReply ()
{
  //- noop
}

// ============================================================================
// AttributeGroupReply::AttributeGroupReply
// ============================================================================
const AttributeGroupReply& AttributeGroupReply::operator= (const AttributeGroupReply& src)
{
  if (this == &src)
    return *this;

  dev_name_m = src.dev_name_m;
  attr_name_m = src.attr_name_m;
  has_failed_m = src.has_failed_m;
  data_m = src.data_m;
  exception_m = src.exception_m;

  return *this;
}

//=============================================================================
// AttributeGroupReplyList::AttributeGroupReplyList
//=============================================================================
AttributeGroupReplyList::AttributeGroupReplyList (size_t n)
  : std::vector<AttributeGroupReply>(n),
    has_failed_m(false)
{
  //- noop
}

//=============================================================================
// AttributeGroupReplyList::~AttributeGroupReplyList
//=============================================================================
AttributeGroupReplyList::~AttributeGroupReplyList ()
{
  //- noop
}

// ============================================================================
// AttributeGroupReply::AttributeGroup
// ============================================================================
AttributeGroup::AttributeGroup (Tango::DeviceImpl* dev)
 : Tango::LogAdapter(dev)
{
  //- YAT4TANGO_TRACE("AttributeGroup::AttributeGroup");
}

// ============================================================================
// AttributeGroupReply::~AttributeGroup
// ============================================================================
AttributeGroup::~AttributeGroup ()
{
  //- YAT4TANGO_TRACE("AttributeGroup::~AttributeGroup");
  this->clear();
}

// ============================================================================
// AttributeGroup::register_attribute
// ============================================================================
void AttributeGroup::register_attribute (const std::string& dev_attr)
{
  //- YAT4TANGO_TRACE("AttributeGroup::register_attribute");

  //- find separator between device and attribute name
  size_t pos = dev_attr.find_last_of('/');

  if ( std::string::npos == pos )
  {
    std::ostringstream s;
    s << "invalid device/attribute syntax: " << dev_attr;
    THROW_DEVFAILED("INVALID_ATTRIBUTE_NAME",
                    s.str().c_str(),
                    "AttributeGroup::register_attribute");
  }

  //- extract device name
  std::string dev_name = dev_attr.substr(0, pos);
  if ( dev_name.empty() )
  {
    std::ostringstream s;
    s << "unexpected empty device name in: " << dev_attr;
    THROW_DEVFAILED("INVALID_DEVICE_NAME",
                    s.str().c_str(),
                    "AttributeGroup::register_attribute");
  }

  //- extract attribute name
  std::string attr_name = dev_attr.substr(pos + 1, dev_attr.size() - pos - 1);
  if ( attr_name.empty() )
  {
    std::ostringstream s;
    s << "unexpected empty attribute name in: " << dev_attr;
    THROW_DEVFAILED("INVALID_ATTRIBUTE_NAME",
                    s.str().c_str(),
                    "AttributeGroup::register_attribute");
  }

  //- if no proxy for <dev_name>...
  Tango::DeviceProxy* dev_proxy = 0;
  if ( m_dev_proxy_map.count(dev_name) == 0 )
  {
    try
    {
      //- ...instanciate it
      dev_proxy = new (std::nothrow) Tango::DeviceProxy(dev_name);
      if ( ! dev_proxy )
        throw std::bad_alloc();
      //- enable transparent reconnection (normaly enabled by default)
      dev_proxy->set_transparency_reconnection(true);
      //- associate device and proxy in map
      m_dev_proxy_map[dev_name] = dev_proxy;
      //- create an empty attribute list for this 'new' device
      m_attr_list_map[dev_name] = AttributeGroup::AttributeList();
      //- store the device name in vector of registered devices
      m_unique_dev_vect.push_back(dev_name);
    }
    catch (const std::bad_alloc&)
    {
      std::ostringstream s;
      s << "Tango::DeviceProxy allocation failed for device " << dev_name;
      THROW_DEVFAILED("MEMORY_ERROR",
                      s.str().c_str(),
                      "AttributeGroup::register_attribute");
    }
    catch (Tango::DevFailed &df)
    {
      std::ostringstream s;
      s << "Tango::DeviceProxy allocation failed for device " << dev_name;
      RETHROW_DEVFAILED(df,
                        "DEVICE_ERROR",
                        s.str().c_str(),
                        "AttributeGroup::register_attribute");
    }
    catch (...)
    {
      std::ostringstream s;
      s << "Tango::DeviceProxy allocation failed for device "
        << dev_name
        << " [unknown exception caught]";
      THROW_DEVFAILED("DEVICE_ERROR",
                      s.str().c_str(),
                      "AttributeGroup::register_attribute");
    }
  }
  else
  {
    dev_proxy = m_dev_proxy_map[dev_name];
    if ( ! dev_proxy )
    {
      std::ostringstream s;
      s << "unexpected null Tango::DeviceProxy found in repository for device "
        << dev_name;
      THROW_DEVFAILED("DEVICE_ERROR",
                      s.str().c_str(),
                      "AttributeGroup::register_attribute");

    }
  }

  //- push the device proxy into the ordered proxy list
  m_ordered_dev_proxy_list.push_back(dev_proxy);

  //- get attribute info
  Tango::AttributeInfoEx attr_info;
  try
  {
    attr_info = dev_proxy->get_attribute_config(attr_name);
  }
  catch (Tango::DevFailed& df)
  {
    std::ostringstream oss;
    oss << "failed to obtain attribute info for "
        << dev_attr
        << " [hint: is it an attribute of the specified device?]";
    RETHROW_DEVFAILED(df,
                      "API_ERROR",
                      oss.str().c_str(),
                      "AttributeGroup::register_attribute");
  }
  catch (...)
  {
    std::ostringstream oss;
    oss << "failed to obtain attribute info for "
        << dev_attr
        << " [unknown exception caught]";
    THROW_DEVFAILED("API_ERROR",
                    oss.str().c_str(),
                    "AttributeGroup::register_attribute");
  }

  //- is the attribute scalar?
  bool is_scalar = attr_info.data_format == Tango::SCALAR;
  if ( ! is_scalar )
  {
    std::ostringstream s;
    s << "invalid attribute specify - " << attr_name << " is not SCALAR";
    THROW_DEVFAILED("DEVICE_ERROR",
                    s.str().c_str(),
                    "AttributeGroup::register_attribute");
  }

  //- store fully qualified attribute name
  m_fqn_vect.push_back(dev_attr);

  //- store dev-name
  m_dev_vect.push_back(dev_name);

  //- store attr-name
  m_att_vect.push_back(attr_name);

  //- push <attr_name> into <dev_name>'s attribute list
  m_attr_list_map[dev_name].push_back(attr_name);

  //- store registration id of the specified attribute
  m_attr_2_regid_map[dev_attr] = m_att_vect.size() - 1;
}

// ============================================================================
// AttributeGroup::register_attributes
// ============================================================================
void AttributeGroup::register_attributes (const std::vector<std::string>& dev_attrs)
{
  for (size_t a = 0; a < dev_attrs.size(); a++)
  {
    this->register_attribute(dev_attrs[a]);
  }
}

// ============================================================================
// AttributeGroup::clear
// ============================================================================
void AttributeGroup::clear ()
{
  //- release device proxies...
  ProxiesIterator cur = m_dev_proxy_map.begin();
  ProxiesIterator end = m_dev_proxy_map.end();
  for (; cur != end; ++cur) {
    delete cur->second;
  }
  //- then clear all containers...
  m_dev_proxy_map.clear();
  m_fqn_vect.clear();
  m_dev_vect.clear();
  m_att_vect.clear();
  m_unique_dev_vect.clear();
  m_ordered_dev_proxy_list.clear();
  m_attr_list_map.clear();
  m_attr_2_regid_map.clear();
}

// ============================================================================
// class: AttributeGroup::attributes_list
// ============================================================================
const std::vector<std::string>& AttributeGroup::attribute_list () const
{
  return m_fqn_vect;
}

// ============================================================================
// class: AttributeGroup::device_proxy_list
// ============================================================================
std::vector<Tango::DeviceProxy*> AttributeGroup::device_proxy_list () /*const*/
{
  std::vector<Tango::DeviceProxy*> proxies;
  ProxiesIterator it = m_dev_proxy_map.begin();
  for (; it != m_dev_proxy_map.end(); ++it)
    proxies.push_back(it->second);
  return proxies;
}

// ============================================================================
// class: AttributeGroup::ordered_device_proxy_list
// ============================================================================
const std::vector<Tango::DeviceProxy*> & AttributeGroup::ordered_device_proxy_list () const
{
  return m_ordered_dev_proxy_list;
}

// ============================================================================
// AttributeGroup::read
// ============================================================================
AttributeGroupReplyList AttributeGroup::read ()
{
  //- YAT4TANGO_TRACE("AttributeGroup::read");

  //- empty lists?
  if ( ! m_dev_vect.size() || ! m_att_vect.size() )
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "the AttributeGroup is either empty or not properly setup",
                    "AttributeGroup::read");
  }

  //- the replies to be returned
  AttributeGroupReplyList agrs(m_att_vect.size());

  //- if data is read from only one device, then read it synchronously
  bool single_device = m_unique_dev_vect.size() == 1;
  if ( single_device )
  {
    //- in this case (single device) the attribute order is naturally conserved
    //-------------------------------------------------------------------------

    //- the device name is the only one stored into <m_unique_dev_vect>
    const std::string dev_name = m_unique_dev_vect[0];

    //- the attributes values
    std::vector<Tango::DeviceAttribute> * values = 0;

    try
    {
      //- get device proxy
      Tango::DeviceProxy * dev_proxy = m_dev_proxy_map[dev_name];
      if ( ! dev_proxy )
      {
        std::ostringstream s;
        s << "unexpected null Tango::DeviceProxy found in repository for device "
          << dev_name;
        THROW_DEVFAILED("DEVICE_ERROR",
                        s.str().c_str(),
                        "AttributeGroup::read");
      }

      //- only one device involved so attribute list to read is simply <m_att_vect>
      values = dev_proxy->read_attributes(m_att_vect);
    }
    catch (Tango::DevFailed &df)
    {
      delete values;
      RETHROW_DEVFAILED(df,
                        "DEVICE_ERROR",
                        "Tango exception caught while trying to read attributes",
                        "AttributeGroup::read");
    }
    catch(...)
    {
      delete values;
      THROW_DEVFAILED("DEVICE_ERROR",
                      "unknown exception caught while trying to read attributes",
                      "AttributeGroup::read");
    }

    //- store replies into the returned AttributeGroupReplyList
    for (size_t a = 0; a < values->size(); a++)
    {
      Tango::DeviceAttribute& da = (*values)[a];
      agrs[a].dev_name_m = dev_name;
      agrs[a].attr_name_m = da.name;
      if ( da.has_failed() )
      {
        agrs.has_failed_m = true;
        agrs[a].has_failed_m = true;
        agrs[a].exception_m = Tango::DevFailed(da.get_err_stack());
      }
      else
      {
        agrs[a].has_failed_m = false;
        agrs[a].data_m = da;
      }
    }

    //- avoid memory leaks!
    delete values;
  }
  //- if data is read from several devices, then read it asynchronously
  else
  {
    //- in this case (several devices) the attribute order is NOT naturally conserved
    //- we use <m_attr_2_regid_map> for the right reply order to be guaranteed
    //-------------------------------------------------------------------------------

    //- asynchronous requests identifiers
    std::vector<long> async_req_id;

    //- read attribute list for each registered device
    for (size_t d = 0; d < m_unique_dev_vect.size(); d++)
    {
      //- the device name
      const std::string dev_name = m_unique_dev_vect[d];

      //- select device proxy associated to current device
      Tango::DeviceProxy * dev_proxy = m_dev_proxy_map[dev_name];
      if ( ! dev_proxy )
      {
        std::ostringstream s;
        s << "unexpected null Tango::DeviceProxy found in repository for device "
          << dev_name;
        THROW_DEVFAILED("DEVICE_ERROR",
                        s.str().c_str(),
                        "AttributeGroup::read");
      }

      //- get associated attribute list
      AttributeList & al = m_attr_list_map[dev_name];
      if ( ! al.size() )
      {
        std::ostringstream s;
        s << "unexpected empty attribute list for device "
          << dev_name;
        THROW_DEVFAILED("DEVICE_ERROR",
                        s.str().c_str(),
                        "AttributeGroup::read");
      }

      //- read attributes asynchronously
      try
      {
        long ari = dev_proxy->read_attributes_asynch(al);
        async_req_id.push_back(ari);
      }
      catch (Tango::DevFailed &df)
      {
        RETHROW_DEVFAILED(df,
                          "DEVICE_ERROR",
                          "Tango exception caught while trying to read attributes",
                          "AttributeGroup::read");
      }
      catch (...)
      {
        THROW_DEVFAILED("DEVICE_ERROR",
                        "unknown exception caught while trying to read attributes",
                        "AttributeGroup::read");
      }
    }

    //- get read replies
    for (size_t d = 0; d < m_unique_dev_vect.size(); d++)
    {
      //- the device name
      const std::string dev_name = m_unique_dev_vect[d];

      //- select device proxy associated to current device
      Tango::DeviceProxy * dev_proxy = m_dev_proxy_map[dev_name];

      //- the attribute values
      vector<Tango::DeviceAttribute> * values = 0;

      try
      {
        values = dev_proxy->read_attributes_reply(async_req_id[d], 2500);
      }
      catch (Tango::DevFailed &df)
      {
        delete values;
        RETHROW_DEVFAILED(df,
                          "DEVICE_ERROR",
                          "Tango exception caught while trying to obtain async read replies",
                          "AttributeGroup::read");
      }
      catch (...)
      {
        delete values;
        THROW_DEVFAILED("DEVICE_ERROR",
                        "unknown exception caught while trying to obtain async read replies",
                        "AttributeGroup::read");
      }

      //- store replies into the returned AttributeGroupReplyList
      for (size_t a = 0; a < values->size(); a++)
      {
        //- attribute value
        Tango::DeviceAttribute& da = (*values)[a];
        //- fully qualified attribute name
        std::ostringstream fqn;
        fqn <<  dev_name << "/" << da.name;
        //- registration id of this attribute
        size_t reg_id = m_attr_2_regid_map[fqn.str()];
        if ( reg_id >= agrs.size() )
        {
          delete values;
          std::ostringstream s;
          s << "invalid registration identifier found for "
            << fqn.str();
          THROW_DEVFAILED("DEVICE_ERROR",
                          s.str().c_str(),
                          "AttributeGroup::read");
        }
        agrs[reg_id].dev_name_m = dev_name;
        agrs[reg_id].attr_name_m = da.name;
        if ( da.has_failed() )
        {
          agrs.has_failed_m = true;
          agrs[reg_id].has_failed_m = true;
          agrs[reg_id].exception_m = Tango::DevFailed(da.get_err_stack());
        }
        else
        {
          agrs[reg_id].has_failed_m = false;
          agrs[reg_id].data_m = da;
        }
      }

      //- avoid memory leaks!
      delete values;
      values = 0;
    }
  }

  //- finally... return the replies
  return agrs;
}

} // namespace

