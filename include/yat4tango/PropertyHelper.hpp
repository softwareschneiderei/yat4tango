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
//    Nicolas Leclercq
//    Synchrotron SOLEIL
//----------------------------------------------------------------------------
/*!
 * \authors See AUTHORS file
 */

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <iostream>
#include <limits>
#include <yat/threading/Mutex.h>

namespace yat4tango
{

//-------------------------------------------------------------------------
// Comparison functors specialization for floatting point numbers
//-------------------------------------------------------------------------
template<>
struct PropertyHelper::ValueComp<float>
{
  bool operator()(const float& v1, const float& v2)
  {
    return yat::fp_is_equal(v1, v2, std::numeric_limits<float>::epsilon());
  }
};

template<>
struct PropertyHelper::ValueComp<double>
{
  bool operator()(const double& v1, const double& v2)
  {
    return yat::fp_is_equal(v1, v2, std::numeric_limits<double>::epsilon());
  }
};

template<>
struct PropertyHelper::ValueComp< std::vector<float> >
{
  bool operator()(const std::vector<float>& v1, const std::vector<float>& v2)
  {
    if( v1.size() != v2.size() )
      return false;

    for( std::size_t i = 0; i < v1.size(); ++i )
    {
      if( !yat::fp_is_equal(v1[i], v2[i], std::numeric_limits<float>::epsilon()) )
        return false;
    }
    return true;
  }
};

template<>
struct PropertyHelper::ValueComp< std::vector<double> >
{
  bool operator()(const std::vector<double>& v1, const std::vector<double>& v2)
  {
    if( v1.size() != v2.size() )
      return false;

    for( std::size_t i = 0; i < v1.size(); ++i )
    {
      if( !yat::fp_is_equal(v1[i], v2[i], std::numeric_limits<double>::epsilon()) )
        return false;
    }
    return true;
  }
};

//-------------------------------------------------------------------------
// PropertyHelper::value_cache
//-------------------------------------------------------------------------
template <class T>
bool PropertyHelper::value_cache(Tango::DeviceImpl* dev_p,
                                 const std::string& name, T val,
                                 std::map<DevAttrPair, T>* values_cache_p,
                                 bool store)
{
  typedef typename std::map<DevAttrPair, T>::const_iterator MapIterator;
  static yat::Mutex s_mtx;

  // look for last memorized value
  DevAttrPair key = std::make_pair(dev_p, name);

  if( store )
  {
    yat::AutoMutex<> lock(s_mtx);
    (*values_cache_p)[key] = val;
    return false;
  }
  else
  {
    MapIterator it;
    T last_val;
    {
      yat::AutoMutex<> lock(s_mtx);
      it = (*values_cache_p).find(key);
      if( (*values_cache_p).end() == it )
      {
        // Value not cached yet
        return true;
      }
      else
        last_val = it->second;
    }

    if( PropertyHelper::ValueComp<T>()(last_val, val) )
      return false;

    // The requested val is different than the old one => update cache
    return true;
  }
}

//-------------------------------------------------------------------------
// PropertyHelper::cache_property_value
//-------------------------------------------------------------------------
template <class T>
bool PropertyHelper::cache_property_value(Tango::DeviceImpl* dev_p,
                                          const std::string& property_name,
                                          T val, bool store)
{
  typedef typename std::map<DevAttrPair, T> LastMemValues;
  static LastMemValues s_memorized_values;

  return value_cache(dev_p, property_name, val, &s_memorized_values, store);
}

//-------------------------------------------------------------------------
// PropertyHelper::set_property
//-------------------------------------------------------------------------
template <class T>
void PropertyHelper::set_property(Tango::DeviceImpl* dev_p, const std::string& property_name, T value)
{
  if (!Tango::Util::instance()->_UseDb)
  {
    //- rethrow exception
    Tango::Except::throw_exception(static_cast<const char*>("TANGO_DEVICE_ERROR"),
                                   static_cast<const char*>("NO DB"),
                                   static_cast<const char*>("PropertyHelper::set_property"));
  }

  if( cache_property_value(dev_p, property_name, value, false) )
  {
    Tango::DbDatum current_value(property_name);
    current_value << value;
    Tango::DbData db_data;
    db_data.push_back(current_value);
    try
    {
      dev_p->get_db_device()->put_property(db_data);
      // Store value in cache
      cache_property_value(dev_p, property_name, value, true);
    }
    catch (Tango::DevFailed &df)
    {
      //- rethrow exception
      Tango::Except::re_throw_exception(df,
                                        static_cast<const char*>("TANGO_DEVICE_ERROR"),
                                        static_cast<const char*>(std::string(df.errors[0].desc).c_str()),
                                        static_cast<const char*>("PropertyHelper::set_property"));
    }
  }
}

//-------------------------------------------------------------------------
// PropertyHelper::get_property
//-------------------------------------------------------------------------
template <class T>
T PropertyHelper::get_property(Tango::DeviceImpl* dev_p, const std::string& property_name)
{
  if (!Tango::Util::instance()->_UseDb)
  {
    //- rethrow exception
    Tango::Except::throw_exception(static_cast<const char*>("TANGO_DEVICE_ERROR"),
                                   static_cast<const char*>("NO DB"),
                                   static_cast<const char*>("PropertyHelper::get_property"));
  }

  T value;
  Tango::DbDatum current_value(property_name);
  Tango::DbData db_data;
  db_data.push_back(current_value);
  try
  {
    dev_p->get_db_device()->get_property(db_data);
  }
  catch (Tango::DevFailed &df)
  {
    //- rethrow exception
    Tango::Except::re_throw_exception(df,
                                      static_cast<const char*>("TANGO_DEVICE_ERROR"),
                                      static_cast<const char*>(std::string(df.errors[0].desc).c_str()),
                                      static_cast<const char*>("PropertyHelper::get_property"));
  }
  db_data[0] >> value;
  cache_property_value(dev_p, property_name, value, true);
  return value;
}

//-------------------------------------------------------------------------
// PropertyHelper::create_property_if_empty
//-------------------------------------------------------------------------
template <class T>
void PropertyHelper::create_property_if_empty(Tango::DeviceImpl* dev_p,
                                              Tango::DbData& dev_prop,
                                              T value, string property_name)
{
  std::size_t iNbProperties = dev_prop.size();
  std::size_t i;
  for (i = 0; i < iNbProperties; i++)
  {
    std::string sPropertyName(dev_prop[i].name);
    if (sPropertyName == property_name)
      break;
  }
  if (i == iNbProperties)
    //# TODO: throwing a DevFailed Exception
    return;

  int iPropertyIndex = i;

  if (dev_prop[iPropertyIndex].is_empty())
  {
    Tango::DbDatum current_value(dev_prop[iPropertyIndex].name);
    current_value << value;
    Tango::DbData db_data;
    db_data.push_back(current_value);

    try
    {
      dev_p->get_db_device()->put_property(db_data);
      // Store value in cache
      cache_property_value(dev_p, property_name, value, true);
    }
    catch (Tango::DevFailed &df)
    {
      //- rethrow exception
      Tango::Except::re_throw_exception(df,
                                        static_cast<const char*>("TANGO_DEVICE_ERROR"),
                                        static_cast<const char*>(std::string(df.errors[0].desc).c_str()),
                                        static_cast<const char*>("PropertyHelper::create_property_if_empty"));
    }
  }
}

#define PROP_VALUE_NAME "__value"

//-------------------------------------------------------------------------
// PropertyHelper::cache_memorized_attribute
//-------------------------------------------------------------------------
template <class T>
bool PropertyHelper::cache_memorized_attribute(Tango::DeviceImpl* dev_p,
                                               const std::string& attribute_name,
                                               T val, bool store)
{
  typedef typename std::map<DevAttrPair, T> LastMemValues;
  static LastMemValues s_memorized_values;

  return value_cache(dev_p, attribute_name, val, &s_memorized_values, store);
}

//-------------------------------------------------------------------------
// PropertyHelper::set_memorized_attribute
//-------------------------------------------------------------------------
template <class T>
void PropertyHelper::set_memorized_attribute(Tango::DeviceImpl* dev_p,
                                             const std::string& attribute_name, T val)
{
  if (!Tango::Util::instance()->_UseDb)
  {
    //- rethrow exception
    Tango::Except::throw_exception(static_cast<const char*>("TANGO_DEVICE_ERROR"),
                                   static_cast<const char*>("NO DB"),
                                   static_cast<const char*>("PropertyHelper::set_memorized_attribute"));
  }

  // Check value from cache or database
  if( cache_memorized_attribute(dev_p, attribute_name, val, false) )
  {
    Tango::DbData properties;

    Tango::DbDatum prop_attr_name(attribute_name);
    prop_attr_name << 1L;
    properties.push_back(prop_attr_name);

    Tango::DbDatum prop_name( PROP_VALUE_NAME );
    prop_name << val;

    properties.push_back(prop_name);

    try
    {
      dev_p->get_db_device()->put_attribute_property(properties);
      // Store value in cache
      cache_memorized_attribute(dev_p, attribute_name, val, true);
    }
    catch (Tango::DevFailed &df)
    {
      //- rethrow exception
      Tango::Except::re_throw_exception(df,
                                        static_cast<const char*>("TANGO_DEVICE_ERROR"),
                                        static_cast<const char*>(std::string(df.errors[0].desc).c_str()),
                                        static_cast<const char*>("PropertyHelper::create_property_if_empty"));
    }
  }
}

//-------------------------------------------------------------------------
// PropertyHelper::get_memorized_attribute
//-------------------------------------------------------------------------
template <class T>
T PropertyHelper::get_memorized_attribute(Tango::DeviceImpl* dev_p,
                                          const std::string& attribute_name)
{
  if (!Tango::Util::instance()->_UseDb)
  {
    //- rethrow exception
    Tango::Except::throw_exception(static_cast<const char*>("TANGO_DEVICE_ERROR"),
                                   static_cast<const char*>("NO DB"),
                                   static_cast<const char*>("PropertyHelper::get_memorized_attribute"));
  }

  T value;
  Tango::DbData dev_prop;
  dev_prop.push_back(Tango::DbDatum(attribute_name));

  try
  {
    dev_p->get_db_device()->get_attribute_property(dev_prop);
    for( std::size_t i=0; i < dev_prop.size(); ++i )
    {
      dev_prop[i] >> value;
      if( dev_prop[i].name == PROP_VALUE_NAME )
      {
        cache_memorized_attribute(dev_p, attribute_name, value, true);
        return value;
      }
    }
    Tango::Except::throw_exception( "NO_DATA", "No value", "PropertyHelper::get_property" );
  }
  catch (Tango::DevFailed &df)
  {
    //- rethrow exception
    Tango::Except::re_throw_exception(df,
                                      static_cast<const char*>("TANGO_DEVICE_ERROR"),
                                      static_cast<const char*>(std::string(df.errors[0].desc).c_str()),
                                      static_cast<const char*>("PropertyHelper::get_property"));
  }
  catch( const std::exception& e )
  {
    Tango::Except::throw_exception( "SYSTEM_ERROR", e.what(), "PropertyHelper::get_property" );
  }
  catch( ... )
  {
    Tango::Except::throw_exception( "UNKNOWN_ERROR", "Unknown error", "PropertyHelper::get_property" );
  }
  return value; // unreachable statement but gcc is mistaken by the Tango::Except::throw_exception call
}

//-------------------------------------------------------------------------
// PropertyHelper::get_memorized_attribute
//-------------------------------------------------------------------------
template <class T>
T PropertyHelper::get_memorized_attribute(Tango::DeviceImpl* dev_p,
                                          const std::string& attribute_name, T default_value)
{
  if (!Tango::Util::instance()->_UseDb)
  {
    //- rethrow exception
    Tango::Except::throw_exception(static_cast<const char*>("TANGO_DEVICE_ERROR"),
                                   static_cast<const char*>("NO DB"),
                                   static_cast<const char*>("PropertyHelper::get_memorized_attribute"));
  }

  T value;
  Tango::DbData dev_prop;
  dev_prop.push_back(Tango::DbDatum(attribute_name));

  try
  {
    dev_p->get_db_device()->get_attribute_property(dev_prop);

    for( std::size_t i=0; i < dev_prop.size(); ++i )
    {
      dev_prop[i] >> value;
      if( dev_prop[i].name == PROP_VALUE_NAME )
      {
        cache_memorized_attribute(dev_p, attribute_name, value, true);
        return value;
      }
    }
    return default_value;
  }
  catch (Tango::DevFailed &df)
  {
    //- rethrow exception
    Tango::Except::re_throw_exception(df,
                                      static_cast<const char*>("TANGO_DEVICE_ERROR"),
                                      static_cast<const char*>(std::string(df.errors[0].desc).c_str()),
                                      static_cast<const char*>("PropertyHelper::get_property"));
  }
  catch( const std::exception& e )
  {
    Tango::Except::throw_exception( "SYSTEM_ERROR", e.what(), "PropertyHelper::get_property" );
  }
  catch( ... )
  {
    Tango::Except::throw_exception( "UNKNOWN_ERROR", "Unknown error", "PropertyHelper::get_property" );
  }
  return value; // unreachable statement but gcc is mistaken by the Tango::Except::throw_exception call
}

} // namespace
