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
#include <iterator>
#include <yat4tango/DevicePlugInHelper.h>

namespace yat4tango
{

// ============================================================================
// DevicePlugInHelper::DevicePlugInHelper
// ============================================================================
DevicePlugInHelper::DevicePlugInHelper(Tango::DeviceImpl* hostDevice,
                                        yat4tango::DynamicAttributeManager* dynAttrManager)
  : m_hostDevice(hostDevice),
    m_dynAttrMger(dynAttrManager)
{
  //- noop
}

// ============================================================================
// DevicePlugInHelper::~DevicePlugInHelper
// ============================================================================
DevicePlugInHelper::~DevicePlugInHelper()
{
  //- noop
}

// ============================================================================
// DevicePlugInHelper::register_plugin
// ============================================================================
void DevicePlugInHelper::register_plugin(yat4tango::DevicePlugIn* object)
{
  try
  {
    this->register_properties(object);
  }
  catch(Tango::DevFailed& ex)
  {
    RETHROW_DEVFAILED(ex,
                      "SOFTWARE_FAILURE",
                      "Couldn't set plugin properties - Tango error",
                      "DevicePlugInHelper::register_plugin");
  }
  catch(...)
  {
    THROW_DEVFAILED("UNKNOWN_ERROR",
                    "Couldn't set plugin properties - Unknown error",
                    "DevicePlugInHelper::register_plugin");
  }

  try
  {
    this->register_attributes(object);
  }
  catch(Tango::DevFailed& ex)
  {
    RETHROW_DEVFAILED(ex,
                      "SOFTWARE_FAILURE",
                      "Couldn't set plugin attributes - Tango error",
                      "DevicePlugInHelper::register_plugin");
  }
  catch(...)
  {
    THROW_DEVFAILED("UNKNOWN_ERROR",
                    "Couldn't set plugin attributes - Unknown error",
                    "DevicePlugInHelper::register_plugin");
  }

}

// ============================================================================
// DevicePlugInHelper::unregister_plugin
// ============================================================================
void DevicePlugInHelper::unregister_plugin(yat4tango::DevicePlugIn* object)
{
  yat4tango::DynamicAttributeDescriptionList attrList;

  if (NULL == object)
  {
    THROW_DEVFAILED("SOFTWARE_FAILURE",
                    "Device plugin to unregister is not valid",
                    "DevicePlugInHelper::unregister_plugin");
  }

  // Get the list of dynamic attributes defined by the plug-in.
  try
  {
    object->enumerate_attributes(attrList);
  }
  catch(Tango::DevFailed& ex)
  {
    RETHROW_DEVFAILED(ex,
                      "SOFTWARE_FAILURE",
                      "Error while enumerating plugin attributes",
                      "DevicePlugInHelper::unregister_plugin");
  }
  catch(...)
  {
    THROW_DEVFAILED("UNKNOWN_ERROR",
                    "Error while enumerating plugin attributes",
                    "DevicePlugInHelper::unregister_plugin");
  }

  // Attribute list is empty, no dynamic attribute to delete.
  if (attrList.empty())
  {
    return;
  }

  if (NULL == this->m_dynAttrMger)
  {
    THROW_DEVFAILED("SOFTWARE_FAILURE",
                    "Dynamic attribute manager is not valid",
                    "DevicePlugInHelper::unregister_plugin");
  }

  // Parse attribute list, get attribute name and remove it from the manager
  try
  {
    yat4tango::DynamicAttributeDescriptionList::iterator attrIter;

    for (attrIter = attrList.begin(); attrIter < attrList.end(); attrIter++)
    {
      DynamicAttributeInfo dai = (*attrIter);
      this->m_dynAttrMger->remove_attribute(dai.tai.name);
    }
  }
  catch(Tango::DevFailed& ex)
  {
    RETHROW_DEVFAILED(ex,
                      "SOFTWARE_FAILURE",
                      "Error while deleting plugin attributes",
                      "DevicePlugInHelper::unregister_plugin");
  }
  catch(...)
  {
    THROW_DEVFAILED("UNKNOWN_ERROR",
                    "Error while deleting plugin attributes",
                    "DevicePlugInHelper::unregister_plugin");
  }
}

// ============================================================================
// DevicePlugInHelper::register_properties
// ============================================================================
void DevicePlugInHelper::register_properties(yat4tango::DevicePlugIn* object)
{
  // Get the list of properties needed by the plug-in.
  yat4tango::PlugInPropertyInfoList propInfos;

  if (NULL == object)
  {
    THROW_DEVFAILED("SOFTWARE_FAILURE",
                    "Device plugin to register is not valid",
                    "DevicePlugInHelper::register_properties");
  }

  try
  {
    object->enumerate_properties(propInfos);
  }
  catch(Tango::DevFailed& ex)
  {
    RETHROW_DEVFAILED(ex,
                      "SOFTWARE_FAILURE",
                      "Error while enumerating plugin properties",
                      "DevicePlugInHelper::register_properties");
  }
  catch(...)
  {
    THROW_DEVFAILED("UNKNOWN_ERROR",
                    "Error while enumerating plugin properties",
                    "DevicePlugInHelper::register_properties");
  }

  // Property list is empty, no property to set.
  if (propInfos.empty())
  {
    return;
  }

  // Parse the plug-in property list, get property names and store them
  // in propertyQuery list.
  // The property order in list is kept.
  yat4tango::PlugInPropertyInfoList::const_iterator inIter;
  Tango::DbData propertyQuery;
  std::insert_iterator<Tango::DbData> outIter(propertyQuery, propertyQuery.begin());

  for (inIter = propInfos.begin(); inIter != propInfos.end(); ++inIter)
  {
    *outIter = Tango::DbDatum( (*inIter).first );
  }

  // Call Tango DataBase and extract property values.
  if (NULL == this->m_hostDevice)
  {
    THROW_DEVFAILED("SOFTWARE_FAILURE",
                    "Host Device is not valid",
                    "DevicePlugInHelper::register_properties");
  }

  try
  {
    this->m_hostDevice->get_db_device()->get_property(propertyQuery);
  }
  catch(Tango::DevFailed& ex)
  {
    RETHROW_DEVFAILED(ex,
                      "SOFTWARE_FAILURE",
                      "Error while getting properties from Tango DataBase",
                      "DevicePlugInHelper::register_properties");
  }
  catch(...)
  {
    THROW_DEVFAILED("UNKNOWN_ERROR",
                    "Error while getting properties from Tango DataBase",
                    "DevicePlugInHelper::register_properties");
  }


  // Copy property values in a yat4tango::PlugInPropertyValueList list.
  // The property order in list is kept.
  yat4tango::PlugInPropertyValueList propValueList;
  Tango::DbData::iterator propertyQueryIter;

  try
  {
    int i = -1;
    yat4tango::PlugInPropertyValue val;

    for ( propertyQueryIter  = propertyQuery.begin();
          propertyQueryIter != propertyQuery.end();
          ++propertyQueryIter )
    {
      i++;
      std::string propertyName = (*propertyQueryIter).name;
      val.name = propertyName;

//- Macro for type mapping:
#define YAT4TANGO_PUSH_PLUGIN_PROP_TYPE(TYPEID, TANGO_TYPE, YAT_TYPE) \
      case TYPEID:                                             \
      {                                                      \
        if ( !(*propertyQueryIter).is_empty() )              \
        {                                                    \
          TANGO_TYPE value;                                  \
          (*propertyQueryIter) >> value;                     \
          val.value = static_cast<YAT_TYPE>(value);          \
          val.valid = true;                                  \
        }                                                    \
        else                                                 \
        {                                                    \
          val.valid = false;                                 \
        }                                                    \
      }                                                      \
      break;

#define YAT4TANGO_PUSH_PLUGIN_PROP_VECTOR_TYPE(TYPEID, TANGO_TYPE, YAT_TYPE) \
      case TYPEID:                                               \
      {                                                        \
        if ( !(*propertyQueryIter).is_empty() )                \
        {                                                      \
          std::vector<TANGO_TYPE> tango_value;                 \
          (*propertyQueryIter) >> tango_value;                 \
          std::vector<YAT_TYPE> yat_value;                     \
          for ( size_t i = 0; i < tango_value.size(); i++)     \
            yat_value.push_back(static_cast<YAT_TYPE>(tango_value[i])); \
          val.value = yat_value;                               \
          val.valid = true;                                    \
        }                                                      \
        else                                                   \
        {                                                      \
          val.valid = false;                                   \
        }                                                      \
      }                                                        \
      break;

      switch (propInfos[propertyName])
      {
        YAT4TANGO_PUSH_PLUGIN_PROP_TYPE( yat::PlugInPropType::BOOLEAN, Tango::DevBoolean, bool);
        YAT4TANGO_PUSH_PLUGIN_PROP_TYPE( yat::PlugInPropType::UINT8, Tango::DevUChar, yat::uint8);
        YAT4TANGO_PUSH_PLUGIN_PROP_TYPE( yat::PlugInPropType::INT16, Tango::DevShort, yat::int16);
        YAT4TANGO_PUSH_PLUGIN_PROP_TYPE( yat::PlugInPropType::UINT16, Tango::DevUShort, yat::uint16);
        YAT4TANGO_PUSH_PLUGIN_PROP_TYPE( yat::PlugInPropType::INT32, Tango::DevLong, yat::int32);
        YAT4TANGO_PUSH_PLUGIN_PROP_TYPE( yat::PlugInPropType::UINT32, Tango::DevULong, yat::uint32);
        YAT4TANGO_PUSH_PLUGIN_PROP_TYPE( yat::PlugInPropType::INT64, Tango::DevLong64, yat::int64);
        YAT4TANGO_PUSH_PLUGIN_PROP_TYPE( yat::PlugInPropType::UINT64, Tango::DevULong64, yat::uint64);
        YAT4TANGO_PUSH_PLUGIN_PROP_TYPE( yat::PlugInPropType::FLOAT, Tango::DevFloat, float);
        YAT4TANGO_PUSH_PLUGIN_PROP_TYPE( yat::PlugInPropType::DOUBLE, Tango::DevDouble, double);
        YAT4TANGO_PUSH_PLUGIN_PROP_TYPE( yat::PlugInPropType::STRING, std::string, std::string);
        YAT4TANGO_PUSH_PLUGIN_PROP_VECTOR_TYPE( yat::PlugInPropType::STRING_VECTOR, std::string, std::string);
        YAT4TANGO_PUSH_PLUGIN_PROP_VECTOR_TYPE( yat::PlugInPropType::INT16_VECTOR, Tango::DevShort, yat::int16);
        YAT4TANGO_PUSH_PLUGIN_PROP_VECTOR_TYPE( yat::PlugInPropType::UINT16_VECTOR, Tango::DevUShort, yat::uint16);
        YAT4TANGO_PUSH_PLUGIN_PROP_VECTOR_TYPE( yat::PlugInPropType::INT32_VECTOR, Tango::DevLong, yat::int32);
        YAT4TANGO_PUSH_PLUGIN_PROP_VECTOR_TYPE( yat::PlugInPropType::UINT32_VECTOR, Tango::DevULong, yat::uint32);
        YAT4TANGO_PUSH_PLUGIN_PROP_VECTOR_TYPE( yat::PlugInPropType::INT64_VECTOR, Tango::DevLong64, yat::int64);
        YAT4TANGO_PUSH_PLUGIN_PROP_VECTOR_TYPE( yat::PlugInPropType::UINT64_VECTOR, Tango::DevULong64, yat::uint64);
        YAT4TANGO_PUSH_PLUGIN_PROP_VECTOR_TYPE( yat::PlugInPropType::FLOAT_VECTOR, Tango::DevFloat, float);
        YAT4TANGO_PUSH_PLUGIN_PROP_VECTOR_TYPE( yat::PlugInPropType::DOUBLE_VECTOR, Tango::DevDouble, double);
        default:
        {
          THROW_DEVFAILED("SOFTWARE_FAILURE",
                          "Unsupported property type",
                          "DevicePlugInHelper::register_plugin");
        }
      }

      propValueList.push_back(val);
    } // end for
  }
  catch(Tango::DevFailed& ex)
  {
    RETHROW_DEVFAILED(ex,
                      "SOFTWARE_FAILURE",
                      "Error while extracting plugin properties from database properties",
                      "DevicePlugInHelper::register_properties");
  }
  catch(...)
  {
    THROW_DEVFAILED("UNKNOWN_ERROR",
                    "Error while extracting plugin properties from database properties",
                    "DevicePlugInHelper::register_properties");
  }

  // Send the list filled with the property values to the plug-in.
  try
  {
    object->set_properties(propValueList);
  }
  catch(Tango::DevFailed& ex)
  {
    RETHROW_DEVFAILED(ex,
                      "SOFTWARE_FAILURE",
                      "Error while setting plugin properties",
                      "DevicePlugInHelper::register_properties");
  }
  catch(...)
  {
    THROW_DEVFAILED("UNKNOWN_ERROR",
                    "Error while setting plugin properties",
                    "DevicePlugInHelper::register_properties");
  }
}

// ============================================================================
// DevicePlugInHelper::register_attributes
// ============================================================================
void DevicePlugInHelper::register_attributes(yat4tango::DevicePlugIn* object)
{
  yat4tango::DynamicAttributeDescriptionList attrList;

  if (NULL == object)
  {
    THROW_DEVFAILED("SOFTWARE_FAILURE",
                    "Device plugin to register is not valid",
                    "DevicePlugInHelper::register_attributes");
  }

  // Get the list of dynamic attributes defined by the plug-in.
  try
  {
    object->enumerate_attributes(attrList);
  }
  catch(Tango::DevFailed& ex)
  {
    RETHROW_DEVFAILED(ex,
                      "SOFTWARE_FAILURE",
                      "Error while enumerating plugin attributes",
                      "DevicePlugInHelper::register_attributes");
  }
  catch(...)
  {
    THROW_DEVFAILED("UNKNOWN_ERROR",
                    "Error while enumerating plugin attributes",
                    "DevicePlugInHelper::register_attributes");
  }

  // Attribute list is empty, no dynamic attribute to create.
  if (attrList.empty())
  {
    return;
  }

  // Parse attribute list, fill the Device field and creates the attribute
  if (NULL == this->m_hostDevice)
  {
    THROW_DEVFAILED("SOFTWARE_FAILURE",
                    "Host Device is not valid",
                    "DevicePlugInHelper::register_attributes");
  }

  if (NULL == this->m_dynAttrMger)
  {
    THROW_DEVFAILED("SOFTWARE_FAILURE",
                    "Dynamic attribute manager is not valid",
                    "DevicePlugInHelper::register_attributes");
  }

  try
  {
    yat4tango::DynamicAttributeDescriptionList::iterator attrIter;

    for (attrIter = attrList.begin(); attrIter < attrList.end(); attrIter++)
    {
      DynamicAttributeInfo dai = (*attrIter);
      dai.dev = this->m_hostDevice;
      this->m_dynAttrMger->add_attribute(dai);
    }
  }
  catch(Tango::DevFailed& ex)
  {
    RETHROW_DEVFAILED(ex,
                      "SOFTWARE_FAILURE",
                      "Error while creating plugin attributes",
                      "DevicePlugInHelper::register_attributes");
  }
  catch(...)
  {
    THROW_DEVFAILED("UNKNOWN_ERROR",
                    "Error while creating plugin attributes",
                    "DevicePlugInHelper::register_attributes");
  }
}

} // namespace
