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
#include <yat/threading/Mutex.h>
#include <yat/utils/StringTokenizer.h>
#include <yat4tango/DynamicAttributeManager.h>

namespace yat4tango
{
//==============================================================================
// Class DynamicAttr
//==============================================================================

//-------------------------------------------------------------------------
// DynamicAttr::req_read
//-------------------------------------------------------------------------
void DynamicAttr::req_read(Tango::Attr* ta_p, Tango::DeviceImpl* dev_p, Tango::Attribute& a)
{
  DynamicAttribute *da_p = DynamicAttributeClassRepository::get_dyn_attribute(ta_p, dev_p);

  if( da_p )
    da_p->read(a);
}

//-------------------------------------------------------------------------
// DynamicAttr::req_write
//-------------------------------------------------------------------------
void DynamicAttr::req_write(Tango::Attr* ta_p, Tango::DeviceImpl* dev_p, Tango::WAttribute& a)
{
  DynamicAttribute *da_p = DynamicAttributeClassRepository::get_dyn_attribute(ta_p, dev_p);

  if( da_p )
    da_p->write(a);
}

//-------------------------------------------------------------------------
// DynamicAttr::req_is_allowed
//-------------------------------------------------------------------------
bool DynamicAttr::req_is_allowed(Tango::Attr* ta_p, Tango::DeviceImpl* dev_p, Tango::AttReqType rt)
{
  DynamicAttribute *da_p = DynamicAttributeClassRepository::get_dyn_attribute(ta_p, dev_p);

  if( da_p )
    return da_p->is_allowed(rt);

  return false;
}

// ============================================================================
// set_user_defined_properties
// ============================================================================
void set_user_defined_properties (Tango::Attr& a, const DynamicAttributeInfo& i)
{
  Tango::UserDefaultAttrProp  p;

  if (i.tai.label.size())
    p.set_label( i.tai.label.c_str() );

  if (i.tai.description.size())
    p.set_description( i.tai.description.c_str() );

  if (i.tai.unit.size())
    p.set_unit( i.tai.unit.c_str() );

  if (i.tai.standard_unit.size())
    p.set_standard_unit( i.tai.standard_unit.c_str() );

  if (i.tai.display_unit.size())
    p.set_display_unit( i.tai.display_unit.c_str() );

  if (i.tai.format.size())
    p.set_format( i.tai.format.c_str() );

  if (i.tai.min_value.size())
    p.set_min_value( i.tai.min_value.c_str() );

  if (i.tai.max_value.size())
    p.set_max_value( i.tai.max_value.c_str() );

  if (i.tai.alarms.min_alarm.size())
    p.set_min_alarm( i.tai.alarms.min_alarm.c_str() );

  if (i.tai.alarms.max_alarm.size())
    p.set_max_alarm( i.tai.alarms.max_alarm.c_str() );

  if (i.tai.alarms.min_warning.size())
    p.set_min_warning( i.tai.alarms.min_warning.c_str() );

  if (i.tai.alarms.max_warning.size())
    p.set_max_warning( i.tai.alarms.max_warning.c_str() );

  if (i.tai.alarms.delta_t.size())
    p.set_delta_t( i.tai.alarms.delta_t.c_str() );

  if (i.tai.alarms.delta_val.size())
    p.set_delta_val( i.tai.alarms.delta_val.c_str() );

#if (TANGO_VERSION_MAJOR >= 8)
  if (i.tai.events.per_event.period.size())
    p.set_event_period(i.tai.events.per_event.period.c_str());

  if (i.tai.events.ch_event.abs_change.size())
    p.set_event_abs_change(i.tai.events.ch_event.abs_change.c_str());

  if (i.tai.events.ch_event.rel_change.size())
    p.set_event_rel_change(i.tai.events.ch_event.rel_change.c_str());

  if (i.tai.events.arch_event.archive_period.size())
    p.set_archive_event_period(i.tai.events.arch_event.archive_period.c_str());

  if (i.tai.events.arch_event.archive_abs_change.size())
    p.set_archive_event_abs_change(i.tai.events.arch_event.archive_abs_change.c_str());

  if (i.tai.events.arch_event.archive_rel_change.size())
    p.set_archive_event_rel_change(i.tai.events.arch_event.archive_rel_change.c_str());
#endif

#if (TANGO_VERSION_MAJOR >= 9)
  if (!i.tai.enum_labels.empty())
    p.set_enum_labels(const_cast<std::vector<std::string>&>(i.tai.enum_labels));
#endif

  a.set_default_properties(p);

  if ( i.memorized )
    a.set_memorized();

  if ( i.write_memorized_value_at_init )
    a.set_memorized_init(true);
  else
    a.set_memorized_init(false);

  if ( i.polling_period_in_ms )
    a.set_polling_period(i.polling_period_in_ms);

  a.set_data_ready_event(i.data_ready_pushed_by_user_code);

  a.set_change_event(i.change_evt_pushed_by_user_code, !i.change_evt_managed_by_user_code);

  a.set_archive_event(i.archive_evt_pushed_by_user_code, !i.archive_evt_managed_by_user_code);

}

yat::Mutex DynamicAttributeClassRepository::s_mtx_;

// ============================================================================
// DynamicAttributeClassRepository::register_attribute
// ============================================================================
Tango::Attr* DynamicAttributeClassRepository::register_attribute(DynamicAttribute* da_p)
{
  yat::AutoMutex<> _lock(s_mtx_);

  const DynamicAttributeInfo& dai = da_p->get_dai();
  Tango::DeviceClass* class_p = dai.dev->get_device_class();

  MapClassAttrs::iterator it = instance().class_attr_map_.find(class_p);
  if( it != instance().class_attr_map_.end() )
  {
    const Attrs& attrs = it->second;
    for( std::size_t i = 0; i < attrs.size(); ++i )
    {
      if( yat::String(dai.lan).is_equal_no_case(attrs[i]->get_name()) )
      {
        if( instance().attr_nb_devs_map_.find(attrs[i]) != instance().attr_nb_devs_map_.end() )
          instance().attr_nb_devs_map_[attrs[i]]++;
        else
          instance().attr_nb_devs_map_[attrs[i]] = 1;

        instance().attrs_map_[std::make_pair(attrs[i], dai.dev)] = da_p;
        return attrs[i];
      }
    }
  }

  Tango::Attr *ta_p = 0;
  switch( dai.tai.data_format )
  {
    case Tango::SCALAR:
      ta_p = new DynamicScalarAttr(dai);
      break;
    case Tango::SPECTRUM:
      ta_p = new DynamicSpectrumAttr(dai);
      break;
    case Tango::IMAGE:
      ta_p = new DynamicImageAttr(dai);
      break;
    case Tango::FMT_UNKNOWN:
      throw yat::Exception("ERROR", "Unknown data format", "DynamicAttributeClassRepository::register_attribute");
  }

  set_user_defined_properties(*ta_p, dai);
  instance().class_attr_map_[class_p].push_back(ta_p);
  instance().attr_nb_devs_map_[ta_p] = 1;
  instance().attrs_map_[std::make_pair(ta_p, dai.dev)] = da_p;

   //- add the dynamic attribute to the device interface
  try
  {
    dai.dev->add_attribute(ta_p);
  }
  catch (Tango::DevFailed& ex)
  {
    RETHROW_DEVFAILED(ex,
                      "API_ERROR",
                      yat::Format("couldn't add dynamic attribute <{}>"
                                  " - Tango exception caught").arg(dai.lan).cget(),
                      "DynamicAttributeManager::add_attribute");
  }
  catch (...)
  {
    THROW_DEVFAILED("API_ERROR",
                    yat::Format("couldn't add dynamic attribute <{}>"
                                " - Unknown exception caught").arg(dai.lan).cget(),
                    "DynamicAttributeManager::add_attribute");
  }

  return ta_p;
}

// ============================================================================
// DynamicAttributeClassRepository::unregister_attribute
// ============================================================================
void DynamicAttributeClassRepository::unregister_attribute(Tango::Attr* attr_p,
                                                           Tango::DeviceImpl* dev_p)
{
  yat::AutoMutex<> _lock(s_mtx_);

  AttrDev key = std::make_pair(attr_p, dev_p);
  AttrMap::iterator it1 = instance().attrs_map_.find(key);
  if( it1 != instance().attrs_map_.end() )
  {
    DynamicAttribute* dyn_attr_p = it1->second;

    AttrNbDevs::iterator it2 = instance().attr_nb_devs_map_.find(attr_p);
    if( it2 != instance().attr_nb_devs_map_.end() )
    {
      --instance().attr_nb_devs_map_[attr_p];
      if( 0 == instance().attr_nb_devs_map_[attr_p] )
      {
        //- this Tango::Attr instance is no longer attached to a device
        Tango::DeviceClass* class_p = dev_p->get_device_class();
        MapClassAttrs::iterator it3 = instance().class_attr_map_.find(class_p);
        if( it3 != instance().class_attr_map_.end() )
        {
          //- search it in the class-attr mapping
          Attrs& attrs = it3->second;
          for( Attrs::iterator it4 = attrs.begin(); it4 != attrs.end(); ++it4 )
          {
            if( *it4 == attr_p )
            {
              //- remove it
              attrs.erase(it4);
              break;
            }
          }
          if( attrs.size() == 0 )
            //- remove the relation between Tango::DeviceClass and
            //  vector<Tango::Attr> which is now empty
            instance().class_attr_map_.erase(it3);
        }
        //- remove the devices counter for the Tango::Attr
        instance().attr_nb_devs_map_.erase(it2);

        //- remove the Tango::Attr from the device's class interface
    #if (TANGO_VERSION_MAJOR >= 8)
        bool cdb = dyn_attr_p->get_dai().memorized ? false : dyn_attr_p->get_dai().cdb;
        dev_p->remove_attribute(attr_p, true, cdb);
    #else
        dev_p->remove_attribute(attr_p, true);
    #endif
      }
    }

    //- remove the mapping between (TangoAttr, TangoDeviceImpl) and DynamicAttribute
    instance().attrs_map_.erase(it1);
  }
}

//-------------------------------------------------------------------------
// DynamicAttributeClassRepository::get_dyn_attribute
//-------------------------------------------------------------------------
DynamicAttribute* DynamicAttributeClassRepository::get_dyn_attribute(Tango::Attr* ta_p,
                                                                     Tango::DeviceImpl* dev_p)
{
  yat::AutoMutex<> _lock(s_mtx_);

  AttrDev key = std::make_pair(ta_p, dev_p);
  AttrMap::const_iterator cit = instance().attrs_map_.find(key);
  if( cit != instance().attrs_map_.end() )
  {
    return cit->second;
  }
  else
    return NULL;
}

// ============================================================================
// DynamicAttributeManager::ctor
// ============================================================================
DynamicAttributeManager::DynamicAttributeManager(Tango::DeviceImpl * _dev)
  : TangoLogAdapter(_dev), dev_(_dev)
{
  YAT4TANGO_TRACE("DynamicAttributeManager::DynamicAttributeManager");
}

// ============================================================================
// DynamicAttributeManager::dtor
// ============================================================================
DynamicAttributeManager::~DynamicAttributeManager()
{
  YAT4TANGO_TRACE("DynamicAttributeManager::~DynamicAttributeManager");

  try
  {
    this->remove_attributes();
  }
  catch( Tango::DevFailed& e)
  {
    TANGO_EXCEPTION_TO_LOG_STREAM(e);
  }
  catch(...)
  {
    error("Unknown error occured while removing dynamic attributes!!!");
  }
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
                     "DynamicAttributeManager::set_host_device");
  }

  if ( ! h )
  {
    THROW_DEVFAILED( "INVALID_ARG",
                     "unvalid Tango::DeviceImpl specified [unexcpected null pointer]",
                     "DynamicCommandManager::set_host_device");
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

  Tango::Attr* ta = da->get_tango_attribute();
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
    DynamicAttribute* da = (*it).second;
    DynamicAttributeClassRepository::unregister_attribute(da->get_tango_attribute(), dev_);
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
      DynamicAttribute* da = (*it).second;
      DynamicAttributeClassRepository::unregister_attribute(da->get_tango_attribute(), dev_);
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


