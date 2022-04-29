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
#include <limits>
#include <algorithm>
#if defined(WIN32)
# include <cctype>
#endif
#include "yat/utils/StringTokenizer.h"
#include "yat4tango/DynamicAttribute.h"
#include <devapi.h>

namespace yat4tango
{

//- used as default value for 'local' attribute names
const std::string KeepOriginalAttributeName = "_K_O_A_N_";

// ============================================================================
// DynamicAttributeInfo::DynamicAttributeInfo
// ============================================================================
DynamicAttributeInfo::DynamicAttributeInfo ()
  : dev(0),
    lan(yat4tango::KeepOriginalAttributeName),
    tai(),
    rcb(0),
    wcb(0),
    memorized(false),
    write_memorized_value_at_init(false),
    polling_period_in_ms(0),
    cdb(true),
    data_ready_pushed_by_user_code(false),
    change_evt_pushed_by_user_code(false),
    change_evt_managed_by_user_code(false),
    archive_evt_pushed_by_user_code(false),
    archive_evt_managed_by_user_code(false),
    uad(0)
{
  //- std::cout << "DynamicAttributeInfo::dflt-ctor" << std::endl;

  tai.max_dim_x = static_cast<int>(LONG_MAX);
  tai.max_dim_y = static_cast<int>(LONG_MAX);

#if (TANGO_VERSION_MAJOR >= 9)
  tai.memorized = Tango::AttrMemorizedType::NONE;
#endif

}

// ============================================================================
// DynamicAttributeInfo::DynamicAttributeInfo
// ============================================================================
DynamicAttributeInfo::DynamicAttributeInfo (const DynamicAttributeInfo& s)
{
  //- std::cout << "DynamicAttributeInfo::copy-ctor" << std::endl;

  *this = s;
}

// ============================================================================
// DynamicAttributeInfo::operator=
// ============================================================================
const DynamicAttributeInfo& DynamicAttributeInfo::operator= (const DynamicAttributeInfo& s)
{
  //- std::cout << "DynamicAttributeInfo::operator=" << std::endl;

  if (&s == this)
    return *this;

  dev = s.dev;
  tai = s.tai;
  rcb = s.rcb;
  wcb = s.wcb;
  uad = s.uad;
  lan = s.lan;
  cdb = s.cdb;

  memorized = s.memorized;
  write_memorized_value_at_init = s.write_memorized_value_at_init;
  polling_period_in_ms = s.polling_period_in_ms;

  if ( ! tai.max_dim_x )
    tai.max_dim_x = static_cast<int>(LONG_MAX);

  if ( ! tai.max_dim_y )
    tai.max_dim_y = static_cast<int>(LONG_MAX);

  return *this;
}

// ============================================================================
// DynamicAttributeInfo::get_tango_data_type_str
// ============================================================================
const char * DynamicAttributeInfo::get_tango_data_type_str () const
{
  static const char * __str__[] =
  {
    "DEV_VOID",
    "DEV_BOOLEAN",
    "DEV_SHORT",
    "DEV_LONG",
    "DEV_FLOAT",
    "DEV_DOUBLE",
    "DEV_USHORT",
    "DEV_ULONG",
    "DEV_STRING",
    "DEVVAR_CHARARRAY",
    "DEVVAR_SHORTARRAY",
    "DEVVAR_LONGARRAY",
    "DEVVAR_FLOATARRAY",
    "DEVVAR_DOUBLEARRAY",
    "DEVVAR_USHORTARRAY",
    "DEVVAR_ULONGARRAY",
    "DEVVAR_STRINGARRAY",
    "DEVVAR_LONGSTRINGARRAY",
    "DEVVAR_DOUBLESTRINGARRAY",
    "DEV_STATE",
    "CONST_DEV_STRING",
    "DEVVAR_BOOLEANARRAY",
    "DEV_UCHAR",
    "DEV_LONG64",
    "DEV_ULONG64",
    "DEVVAR_LONG64ARRAY",
    "DEVVAR_ULONG64ARRAY",
    "DEV_INT",
    "DEV_ENCODED",
#if (TANGO_VERSION_MAJOR >= 9)
    "DEV_ENUM",
    "DEV_PIPE_BLOB",
    "DEVVAR_STATEARRAY",
#endif
    //-----------
    "UNKNOWN_TANGO_DATA_TYPE"
  };

#if (TANGO_VERSION_MAJOR >= 9)
  static int __unknown_id__ = Tango::DEVVAR_STATEARRAY + 1;
#else
  static int __unknown_id__ = Tango::DEV_ENCODED + 1;
#endif

  int i = this->tai.data_type;

#if (TANGO_VERSION_MAJOR >= 9)
  if (i >= Tango::DEV_VOID && i <= Tango::DEVVAR_STATEARRAY)
#else
  if (i >= Tango::DEV_VOID && i <= Tango::DEV_ENCODED)
#endif
    return __str__[i];

  return __str__[__unknown_id__];
}

// ============================================================================
// DynamicAttributeInfo::get_tango_data_access_str
// ============================================================================
const char * DynamicAttributeInfo::get_tango_data_access_str () const
{
	static const char * __str__[] =
  {
    "READ",
	  "READ_WITH_WRITE",
	  "WRITE",
	  "READ_WRITE",
    //-----------
    "UNKNOWN_TANGO_RW_ACCESS_TYPE"
  };

  static int __unknown_id__ = Tango::READ_WRITE + 1;

  int i = this->tai.writable;

  if (i >= Tango::READ && i <= Tango::READ_WRITE)
    return __str__[i];

  return __str__[__unknown_id__];
}

// ============================================================================
// DynamicAttribute::get_tango_data_format_str
// ============================================================================
const char * DynamicAttributeInfo::get_tango_data_format_str () const
{
	static const char * __str__[] =
  {
    "SCALAR",
	  "SPECTRUM",
	  "IMAGE",
    //-----------
    "UNKNOWN_TANGO_DATA_FORMAT"
  };

  static int __unknown_id__ = Tango::IMAGE + 1;

  int i = this->tai.data_format;

  if (i >= Tango::SCALAR && i <= Tango::IMAGE)
    return __str__[i];

  return __str__[__unknown_id__];
}

// ============================================================================
// DynamicAttributeInfo::dump
// ============================================================================
void DynamicAttributeInfo::dump ()  /*const*/
{
  std::cout << "DynamicAttributeInfo.this........." << this << std::endl;
  std::cout << "DynamicAttributeInfo.loc_name....." << this->lan << std::endl;
  std::cout << "DynamicAttributeInfo.rem_name....." << this->tai.name << std::endl;
  std::cout << "DynamicAttributeInfo.type........." << this->get_tango_data_type_str() << std::endl;
  std::cout << "DynamicAttributeInfo.fmt.........." << this->get_tango_data_format_str() << std::endl;
  std::cout << "DynamicAttributeInfo.r/w.........." << this->get_tango_data_access_str() << std::endl;
  std::cout << "DynamicAttributeInfo.rcb.........." << (this->rcb.is_empty() ? "no" : "yes") << std::endl;
  std::cout << "DynamicAttributeInfo.wcb.........." << (this->wcb.is_empty() ? "no" : "yes") << std::endl;
  std::cout << "DynamicAttributeInfo.uad.........." << this->uad.type().name() << std::endl;
  std::cout << "DynamicAttributeInfo.memorized...." << this->memorized << std::endl;
  std::cout << "DynamicAttributeInfo.wrt@init....." << this->write_memorized_value_at_init << std::endl;
  std::cout << "DynamicAttributeInfo.db-cleanup..." << this->cdb << std::endl;
}

// ============================================================================
// ForwardedAttributeInfo::ForwardedAttributeInfo
// ============================================================================
ForwardedAttributeInfo::ForwardedAttributeInfo ()
  : dev(0), url(), lan(yat4tango::KeepOriginalAttributeName), rdo(false)
{
  //- noop
}

// ============================================================================
// ForwardedAttributeInfo::ForwardedAttributeInfo
// ============================================================================
ForwardedAttributeInfo::ForwardedAttributeInfo (const ForwardedAttributeInfo& s)
{
  *this = s;
}

// ============================================================================
// ForwardedAttributeInfo::
// ============================================================================
const ForwardedAttributeInfo& ForwardedAttributeInfo::operator= (const ForwardedAttributeInfo& s)
{
  if (this == &s) return *this;
  dev = s.dev;
  url = s.url;
  lan = s.lan;
  rdo = s.rdo;
  return *this;
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

// ============================================================================
// DynamicAttribute::DynamicAttribute
// ============================================================================
DynamicAttribute::DynamicAttribute (const DynamicAttributeInfo & i)
 : Tango::LogAdapter(i.dev), dai_(i), dp_(0), da_(0)
{
  //- YAT4TANGO_TRACE("DynamicAttribute::DynamicAttribute");
}

// ============================================================================
// DynamicAttribute::~DynamicAttribute
// ============================================================================
DynamicAttribute::~DynamicAttribute ()
{
  //- YAT4TANGO_TRACE("DynamicAttribute::~DynamicAttribute");

  //- removes attribute properties from the tango db
  //- !!!!!! this->cleanup_properties() !!!! THIS CAUSES A CRASH !!!!!!

  //- release data (if any)
  delete da_;

  //- release device proxy (if any)
  delete dp_;
}

// ============================================================================
// DynamicAttribute::cleanup_properties
// ============================================================================
void DynamicAttribute::cleanup_properties ()
{
  if ( this->dai_.memorized  || ! this->dai_.cdb || ! this->dai_.dev )
    return;

  Tango::DeviceData argin;
  std::vector<std::string> v(2);
  v[0] = this->dp_->name();
  v[1] = this->dai_.tai.name;
  argin << v;

  Tango::Database * db = this->dai_.dev->get_db_device()->get_dbase();
  if ( ! db )
    return;

  try
  {
    Tango::DeviceData argout = db->command_inout("DbDeleteDeviceAttribute", argin);
  }
  catch(Tango::DevFailed& ex)
  {
	  RETHROW_DEVFAILED(ex,
                      "INTERNAL_ERROR",
	                    "unable to delete attribute properties from the database",
                      "DynamicAttribute::cleanup_properties");
  }
  catch(...)
  {
	  THROW_DEVFAILED("UNKNOWN_ERROR",
	                  "unable to delete attribute properties from the database",
                    "DynamicAttribute::cleanup_properties");
  }
}

// ============================================================================
//  DynamicAttribute::set_read_callback
// ============================================================================
void DynamicAttribute::set_read_callback (const DynamicAttributeReadCallback & rcb)
{
  this->dai_.rcb = rcb;
}

// ============================================================================
//  DynamicAttribute::set_write_callback
// ============================================================================
void DynamicAttribute::set_write_callback (const DynamicAttributeWriteCallback & wcb)
{
  this->dai_.wcb = wcb;
}

// ============================================================================
//  DynamicAttribute::link_to_proxy
// ============================================================================
void DynamicAttribute::link_to_proxy (Tango::DeviceProxy * dp)
{
  this->dp_ = dp;
  this->da_ = new (std::nothrow) Tango::DeviceAttribute;
  this->dai_.rcb = yat4tango::DynamicAttributeReadCallback::instanciate(*this, &DynamicAttribute::fwd_attr_read);
  this->dai_.wcb = yat4tango::DynamicAttributeWriteCallback::instanciate(*this, &DynamicAttribute::fwd_attr_write);
}

// ============================================================================
// DynamicAttribute::get_tango_data_type_str
// ============================================================================
const char * DynamicAttribute::get_tango_data_type_str () const
{
	return this->dai_.get_tango_data_type_str();
}

// ============================================================================
// DynamicAttribute::get_tango_data_access_str
// ============================================================================
const char * DynamicAttribute::get_tango_data_access_str () const
{
	return this->dai_.get_tango_data_access_str();
}

// ============================================================================
// DynamicAttribute::get_tango_data_format_str
// ============================================================================
const char * DynamicAttribute::get_tango_data_format_str () const
{
	return this->dai_.get_tango_data_format_str();
}

// ============================================================================
// DynamicAttribute::dump
// ============================================================================
void DynamicAttribute::dump () /*const*/
{
  this->dai_.dump();
}

// ============================================================================
// DynamicAttribute::fwd_attr_set_value
// ============================================================================
#define __SET_RW_VALUES__ \
  switch ( this->da_->get_data_format() ) \
  { \
    case Tango::SCALAR: \
      ta.set_value(data_addr); \
      if (wa) \
        wa->set_write_value(data_addr + 1); \
      break; \
    case Tango::SPECTRUM: \
      ta.set_value(data_addr, rdims.dim_x); \
      if (wa) \
        wa->set_write_value(data_addr + rdims.dim_x, wdims.dim_x); \
      break; \
    case Tango::IMAGE: \
      ta.set_value(data_addr, rdims.dim_x, rdims.dim_y); \
      if (wa) \
        wa->set_write_value(data_addr + ( rdims.dim_x * rdims.dim_y ), wdims.dim_x * wdims.dim_y); \
      break; \
    default: \
      break; \
  }

// ============================================================================
// DynamicAttribute::fwd_attr_read_data
// ============================================================================
void DynamicAttribute::fwd_attr_read_data (Tango::Attribute& ta, Tango::DeviceAttribute& da)
{
  if ( ! this->da_ )
  {
    THROW_DEVFAILED("API_ERROR",
                    "unable to forward attribute value - unexpected null pointer to Tango::DeviceAttribute",
                    "DynamicAttribute::fwd_attr_read_data");
  }

  //- move data into local storage
  *(this->da_) = da;

  Tango::AttributeDimension rdims;
  Tango::AttributeDimension wdims;

  if ( ! this->da_->is_empty() )
  {
    rdims = this->da_->get_r_dimension();
    wdims = this->da_->get_w_dimension();
  }
  else
  {
    rdims.dim_x = rdims.dim_y = 0;
    wdims.dim_x = wdims.dim_y = 0;
  }

  Tango::WAttribute * wa = this->is_writable()
                         ? dynamic_cast<Tango::WAttribute*>(&ta)
                         : 0;

  switch ( this->da_->get_type() )
  {
    case Tango::DEV_BOOLEAN:
      {
        Tango::DevBoolean * data_addr = this->da_->is_empty()
                                      ? 0
                                      : yat4tango::TangoTraits<Tango::DevBoolean>::corba_buffer(*(this->da_)).inout().get_buffer();
        __SET_RW_VALUES__;
      }
      break;
    case Tango::DEV_UCHAR:
      {
        Tango::DevUChar * data_addr = this->da_->is_empty()
                                    ? 0
                                    : yat4tango::TangoTraits<Tango::DevUChar>::corba_buffer(*(this->da_)).inout().get_buffer();
        __SET_RW_VALUES__;
      }
      break;
#if (TANGO_VERSION_MAJOR >= 9)
    case Tango::DEV_ENUM:
#endif
    case Tango::DEV_SHORT:
      {
        Tango::DevShort * data_addr = this->da_->is_empty()
                                    ? 0
                                    : yat4tango::TangoTraits<Tango::DevShort>::corba_buffer(*(this->da_)).inout().get_buffer();
        __SET_RW_VALUES__;
      }
      break;
    case Tango::DEV_USHORT:
      {
        Tango::DevUShort * data_addr = this->da_->is_empty()
                                     ? 0
                                     : yat4tango::TangoTraits<Tango::DevUShort>::corba_buffer(*(this->da_)).inout().get_buffer();
        __SET_RW_VALUES__;
      }
      break;
    case Tango::DEV_LONG:
      {
        Tango::DevLong * data_addr = this->da_->is_empty()
                                   ? 0
                                   : yat4tango::TangoTraits<Tango::DevLong>::corba_buffer(*(this->da_)).inout().get_buffer();
        __SET_RW_VALUES__;
      }
      break;
    case Tango::DEV_ULONG:
      {
        Tango::DevULong * data_addr = this->da_->is_empty()
                                    ? 0
                                    : yat4tango::TangoTraits<Tango::DevULong>::corba_buffer(*(this->da_)).inout().get_buffer();
        __SET_RW_VALUES__;
      }
      break;
    case Tango::DEV_LONG64:
      {
        Tango::DevLong64 * data_addr = this->da_->is_empty()
                                     ? 0
                                     : yat4tango::TangoTraits<Tango::DevLong64>::corba_buffer(*(this->da_)).inout().get_buffer();
        __SET_RW_VALUES__;
      }
      break;
    case Tango::DEV_ULONG64:
      {
        Tango::DevULong64 * data_addr = this->da_->is_empty()
                                      ? 0
                                      : yat4tango::TangoTraits<Tango::DevULong64>::corba_buffer(*(this->da_)).inout().get_buffer();
        __SET_RW_VALUES__;
      }
      break;
    case Tango::DEV_FLOAT:
      {
        Tango::DevFloat * data_addr = this->da_->is_empty()
                                    ? 0
                                    : yat4tango::TangoTraits<Tango::DevFloat>::corba_buffer(*(this->da_)).inout().get_buffer();
        __SET_RW_VALUES__;
      }
      break;
    case Tango::DEV_DOUBLE:
      {
        Tango::DevDouble * data_addr = this->da_->is_empty()
                                     ? 0
                                     : yat4tango::TangoTraits<Tango::DevDouble>::corba_buffer(*(this->da_)).inout().get_buffer();
        __SET_RW_VALUES__;
      }
      break;
    case Tango::DEV_STRING:
      {
        Tango::DevString * data_addr = this->da_->is_empty()
                                     ? 0
                                     : yat4tango::TangoTraits<Tango::DevString>::corba_buffer(*(this->da_)).inout().get_buffer();
        __SET_RW_VALUES__;
      }
      break;
    case Tango::DEV_STATE:
      {
        Tango::DevState * data_addr = this->da_->is_empty()
                                    ? 0
                                    : yat4tango::TangoTraits<Tango::DevState>::corba_buffer(*(this->da_)).inout().get_buffer();
        __SET_RW_VALUES__;
      }
      break;
    case Tango::DEV_ENCODED:
      {
        Tango::DevEncoded * data_addr = this->da_->is_empty()
                                      ? 0
                                      : yat4tango::TangoTraits<Tango::DevEncoded>::corba_buffer(*(this->da_)).inout().get_buffer();
        __SET_RW_VALUES__;
      }
      break;
    default:
      THROW_DEVFAILED("UNSUPPORTED_FEATURE",
                      "the requested dynamic attribute data type is not supported",
                      "DynamicAttribute::fwd_attr_read_data");
      break;
  }

  //- transfer quality
  ta.set_quality(this->da_->get_quality());
}

// ============================================================================
// DynamicAttribute::fwd_attr_read
// ============================================================================
void DynamicAttribute::fwd_attr_read (DynamicAttributeReadCallbackData& cbd)
{
  //- YAT4TANGO_TRACE("DynamicAttribute::fwd_attr_read");

  if ( ! this->dp_ )
  {
    std::ostringstream oss;
    oss << "the forwarded attribute <"
        << cbd.dya->get_attribute_info().name
        << "> is not properly initialized [no valid DeviceProxy]";
    THROW_DEVFAILED("API_ERROR",
                    oss.str().c_str(),
                    "DynamicAttribute::fwd_attr_read");
  }

  //- read <remote> attribute
  Tango::DeviceAttribute da;
  try
  {
    da = this->dp_->read_attribute(const_cast<std::string&>(cbd.dya->get_attribute_info().name));
  }
  catch (Tango::DevFailed& df)
  {
    std::ostringstream oss;
    oss << "failed to read forwarded attribute <"
        << this->dp_->dev_name()
        << "/"
        << cbd.dya->get_attribute_info().name
        << "> [Tango exception caught]";
    RETHROW_DEVFAILED(df,
                     "API_ERROR",
                     oss.str().c_str(),
                     "DynamicAttribute::fwd_attr_read");
  }
  catch (...)
  {
    std::ostringstream oss;
    oss << "failed to read forwarded attribute <"
        << this->dp_->dev_name()
        << "/"
        << cbd.dya->get_attribute_info().name
        << "> [unknown exception caught]";
    THROW_DEVFAILED("API_ERROR",
                   oss.str().c_str(),
                   "DynamicAttribute::fwd_attr_read");
  }

  //- finally, forward the attribute value
  this->fwd_attr_read_data(*cbd.tga, da);
}

// ============================================================================
// DynamicAttribute::fwd_attr_write_data::SCALAR_CASE
// ============================================================================
#define SCALAR_CASE( TYPE_ID, TYPE) \
  case TYPE_ID: \
  { \
    TYPE v; \
    wa.get_write_value(v); \
    da << v; \
  } \
  break;

// ============================================================================
// DynamicAttribute::fwd_attr_write_data::SPECTRUM_CASE
// ============================================================================
#define SPECTRUM_CASE( TYPE_ID_1, TYPE_ID_2, TYPE_1, TYPE_2) \
  case TYPE_ID_1: \
  case TYPE_ID_2: \
  { \
    const TYPE_1 * p; \
    wa.get_write_value(p); \
    _CORBA_ULong l = static_cast<_CORBA_ULong>(wa.get_w_dim_x()); \
    TYPE_2 * d = new (std::nothrow) TYPE_2(l); \
    d->length(l); \
    for (_CORBA_ULong i = 0; i < l; i++) { \
        (*d)[i] = p[i];\
    } \
    da << d; \
    da.dim_x = l; \
    da.dim_y = 0; \
  } \
  break;

// ============================================================================
// DynamicAttribute::fwd_attr_write_data::IMAGE_CASE
// ============================================================================
#define IMAGE_CASE( TYPE_ID_1, TYPE_ID_2, TYPE_1, TYPE_2) \
  case TYPE_ID_1: \
  case TYPE_ID_2: \
  { \
    const TYPE_1 * p; \
    wa.get_write_value(p); \
    _CORBA_ULong lx = static_cast<_CORBA_ULong>(wa.get_w_dim_x()); \
    _CORBA_ULong ly = static_cast<_CORBA_ULong>(wa.get_w_dim_y()); \
    TYPE_2 * d = new (std::nothrow) TYPE_2(lx * ly); \
    d->length(lx * ly); \
    _CORBA_ULong idx = 0; \
    for (_CORBA_ULong j = 0; j < ly; j++) { \
      for (_CORBA_ULong i = 0; i < lx; i++) { \
        idx = i + j * lx; \
        (*d)[idx] = p[idx]; \
      } \
    } \
    da << d; \
    da.dim_x = lx; \
    da.dim_y = ly; \
  } \
  break;

// ============================================================================
// DynamicAttribute::fwd_attr_write_data
// ============================================================================
void DynamicAttribute::fwd_attr_write_data (Tango::WAttribute & wa, Tango::DeviceAttribute& da)
{
  switch ( wa.get_data_format() )
  {
    //- SCALAR ATTRIBUTES
    case Tango::SCALAR:
    {
      switch ( wa.get_data_type() )
      {
        SCALAR_CASE(Tango::DEV_BOOLEAN, Tango::DevBoolean);
        SCALAR_CASE(Tango::DEV_UCHAR,   Tango::DevUChar);
        SCALAR_CASE(Tango::DEV_USHORT,  Tango::DevUShort);
        SCALAR_CASE(Tango::DEV_SHORT,   Tango::DevShort);
#if (TANGO_VERSION_MAJOR >= 9)
        SCALAR_CASE(Tango::DEV_ENUM,    Tango::DevShort);
#endif
        SCALAR_CASE(Tango::DEV_ULONG,   Tango::DevULong);
        SCALAR_CASE(Tango::DEV_LONG,    Tango::DevLong);
        SCALAR_CASE(Tango::DEV_ULONG64, Tango::DevULong64);
        SCALAR_CASE(Tango::DEV_LONG64,  Tango::DevLong64);
        SCALAR_CASE(Tango::DEV_FLOAT,   Tango::DevFloat);
        SCALAR_CASE(Tango::DEV_DOUBLE,  Tango::DevDouble);
        SCALAR_CASE(Tango::DEV_STRING,  Tango::DevString);
        SCALAR_CASE(Tango::DEV_STATE,   Tango::DevState);
        SCALAR_CASE(Tango::DEV_ENCODED, Tango::DevEncoded);
        default:
        {
          THROW_DEVFAILED("UNSUPPORTED_FEATURE",
                          "the requested (scalar) dynamic attribute data type is not supported",
                          "DynamicAttribute::fwd_attr_write_data");
        }
        break;
      }
    }
    break;
    //- SPECTRUM ATTRIBUTES
    case Tango::SPECTRUM:
    {
      switch ( wa.get_data_type() )
      {
        SPECTRUM_CASE(Tango::DEV_BOOLEAN, Tango::DEVVAR_BOOLEANARRAY, Tango::DevBoolean, Tango::DevVarBooleanArray);
        SPECTRUM_CASE(Tango::DEV_UCHAR,   Tango::DEVVAR_CHARARRAY,    Tango::DevUChar,   Tango::DevVarCharArray);
        SPECTRUM_CASE(Tango::DEV_USHORT,  Tango::DEVVAR_USHORTARRAY,  Tango::DevUShort,  Tango::DevVarUShortArray);
        SPECTRUM_CASE(Tango::DEV_SHORT,   Tango::DEVVAR_SHORTARRAY,   Tango::DevShort,   Tango::DevVarShortArray);
        SPECTRUM_CASE(Tango::DEV_ULONG,   Tango::DEVVAR_ULONGARRAY,   Tango::DevULong,   Tango::DevVarULongArray);
        SPECTRUM_CASE(Tango::DEV_LONG,    Tango::DEVVAR_LONGARRAY,    Tango::DevLong,    Tango::DevVarLongArray);
        SPECTRUM_CASE(Tango::DEV_ULONG64, Tango::DEVVAR_ULONG64ARRAY, Tango::DevULong64, Tango::DevVarULong64Array);
        SPECTRUM_CASE(Tango::DEV_LONG64,  Tango::DEVVAR_LONG64ARRAY,  Tango::DevLong64,  Tango::DevVarLong64Array);
        SPECTRUM_CASE(Tango::DEV_FLOAT,   Tango::DEVVAR_FLOATARRAY,   Tango::DevFloat,   Tango::DevVarFloatArray);
        SPECTRUM_CASE(Tango::DEV_DOUBLE,  Tango::DEVVAR_DOUBLEARRAY,  Tango::DevDouble,  Tango::DevVarDoubleArray);
        SPECTRUM_CASE(Tango::DEV_STATE,   0xFFFF,                     Tango::DevState,   Tango::DevVarStateArray);
        //- special case for strings
        case Tango::DEV_STRING:
        case Tango::DEVVAR_STRINGARRAY:
        {
          const Tango::ConstDevString * p;
          wa.get_write_value(p);
          _CORBA_ULong l = static_cast<_CORBA_ULong>(wa.get_w_dim_x());
          Tango::DevVarStringArray * d = new (std::nothrow) Tango::DevVarStringArray(l);
          d->length(l);
          for (size_t i = 0; i < l; i++)
            (*d)[i] = CORBA::string_dup(p[i]);
          da.dim_x = l;
          da.dim_y = 0;
          da << d;
        }
        break;
        default:
        {
          THROW_DEVFAILED("UNSUPPORTED_FEATURE",
                          "the requested spectrum dynamic attribute data type is not supported",
                          "DynamicAttribute::fwd_attr_write_data");
        }
        break;
      }
    }
    break;
    //- IMAGE ATTRIBUTES
    case Tango::IMAGE:
    {
      switch ( wa.get_data_type() )
      {
        IMAGE_CASE(Tango::DEV_BOOLEAN, Tango::DEVVAR_BOOLEANARRAY, Tango::DevBoolean, Tango::DevVarBooleanArray);
        IMAGE_CASE(Tango::DEV_UCHAR,   Tango::DEVVAR_CHARARRAY,    Tango::DevUChar,   Tango::DevVarCharArray);
        IMAGE_CASE(Tango::DEV_USHORT,  Tango::DEVVAR_USHORTARRAY,  Tango::DevUShort,  Tango::DevVarUShortArray);
        IMAGE_CASE(Tango::DEV_SHORT,   Tango::DEVVAR_SHORTARRAY,   Tango::DevShort,   Tango::DevVarShortArray);
        IMAGE_CASE(Tango::DEV_ULONG,   Tango::DEVVAR_ULONGARRAY,   Tango::DevULong,   Tango::DevVarULongArray);
        IMAGE_CASE(Tango::DEV_LONG,    Tango::DEVVAR_LONGARRAY,    Tango::DevLong,    Tango::DevVarLongArray);
        IMAGE_CASE(Tango::DEV_ULONG64, Tango::DEVVAR_ULONG64ARRAY, Tango::DevULong64, Tango::DevVarULong64Array);
        IMAGE_CASE(Tango::DEV_LONG64,  Tango::DEVVAR_LONG64ARRAY,  Tango::DevLong64,  Tango::DevVarLong64Array);
        IMAGE_CASE(Tango::DEV_FLOAT,   Tango::DEVVAR_FLOATARRAY,   Tango::DevFloat,   Tango::DevVarFloatArray);
        IMAGE_CASE(Tango::DEV_DOUBLE,  Tango::DEVVAR_DOUBLEARRAY,  Tango::DevDouble,  Tango::DevVarDoubleArray);
        IMAGE_CASE(Tango::DEV_STATE,   0xFFFF,                     Tango::DevState,   Tango::DevVarStateArray);
        //- special case for strings
        case Tango::DEV_STRING:
        case Tango::DEVVAR_STRINGARRAY:
        {
          const Tango::ConstDevString * p;
          wa.get_write_value(p);
          _CORBA_ULong lx = static_cast<_CORBA_ULong>(wa.get_w_dim_x());
          _CORBA_ULong ly = static_cast<_CORBA_ULong>(wa.get_w_dim_y());
          Tango::DevVarStringArray * d = new (std::nothrow) Tango::DevVarStringArray(lx * ly);
          d->length(lx * ly);
          size_t i ,j, idx;
          for (j = 0; j < ly; j++)
          {
            for (i = 0; i < lx; i++)
            {
              idx = i + j * lx;
              (*d)[idx] = CORBA::string_dup(p[idx]);
            }
          }
          da.dim_x = lx;
          da.dim_y = ly;
          da << d;
        }
        break;
        default:
        {
          THROW_DEVFAILED("UNSUPPORTED_FEATURE",
                          "the requested image dynamic attribute data type is not supported",
                          "DynamicAttribute::fwd_attr_write_data");
        }
        break;
      }
    }
    break;
    default:
    {
      THROW_DEVFAILED("UNSUPPORTED_FEATURE",
                      "the requested dynamic attribute format is not supported",
                      "DynamicAttribute::fwd_attr_write_data");
    }
    break;
  }
}

// ============================================================================
// DynamicAttribute::fwd_attr_write
// ============================================================================
void DynamicAttribute::fwd_attr_write (DynamicAttributeWriteCallbackData& cbd)
{
  //- YAT4TANGO_TRACE("DynamicAttribute::fwd_attr_write");

  if ( ! this->dp_ )
  {
    std::ostringstream oss;
    oss << "the forwarded attribute <"
        << cbd.dya->get_attribute_info().name
        << "> is not properly initialized [no valid DeviceProxy]";
    THROW_DEVFAILED("API_ERROR",
                    oss.str().c_str(),
                    "DynamicAttribute::fwd_attr_write");
  }

  //- get write value
  Tango::DeviceAttribute da;
  da.name = cbd.dya->get_attribute_info().name;

  //- transfer write value from Tango::WAttribute to Tango::DeviceAttribute
  this->fwd_attr_write_data(*(cbd.tga), da);

  //- write <remote> attribute
  try
  {
    this->dp_->write_attribute(da);
  }
  catch (Tango::DevFailed& df)
  {
    std::ostringstream oss;
    oss << "failed to write forwarded attribute <"
        << this->dp_->dev_name()
        << "/"
        << cbd.dya->get_attribute_info().name
        << "> [Tango exception caught]";
    RETHROW_DEVFAILED(df,
                      "API_ERROR",
                      oss.str().c_str(),
                      "DynamicAttribute::fwd_attr_write");
  }
  catch (...)
  {
    std::ostringstream oss;
    oss << "failed to write forwarded attribute <"
        << this->dp_->dev_name()
        << "/"
        << cbd.dya->get_attribute_info().name
        << "> [unknown exception caught]";
    THROW_DEVFAILED("API_ERROR",
                    oss.str().c_str(),
                    "DynamicAttribute::fwd_attr_write");
  }
}

// ============================================================================
// template class DynamicScalarAttribute
// ============================================================================
template <typename T>
class DynamicScalarAttribute : public Tango::Attr, public DynamicAttribute
{
public:
  //---------------------------------------------------------------------------
  //- ctor
  //---------------------------------------------------------------------------
  DynamicScalarAttribute (const DynamicAttributeInfo& i)
    : Tango::Attr (i.lan.c_str(), i.tai.data_type, i.tai.disp_level, i.tai.writable),
      DynamicAttribute (i)
  {
    //- YAT4TANGO_TRACE("DynamicScalarAttribute::DynamicScalarAttribute");
    set_user_defined_properties(*this, i);
  }
  //---------------------------------------------------------------------------
  //- dtor
  //---------------------------------------------------------------------------
  ~DynamicScalarAttribute()
  {
    //- YAT4TANGO_TRACE("DynamicScalarAttribute::~DynamicScalarAttribute");
  }
  //---------------------------------------------------------------------------
  //- returns the underlying Tango::Attr
  //---------------------------------------------------------------------------
  virtual Tango::Attr * get_tango_attribute ()
  {
    return this;
  }
  //---------------------------------------------------------------------------
  //- returns the underlying Tango data type of this DynamicAttribute
  //---------------------------------------------------------------------------
  virtual int get_tango_data_type () const
  {
    return TangoTraits<T>::type_id;
  }
  //---------------------------------------------------------------------------
  //- called by Tango on client <read> request
  //---------------------------------------------------------------------------
  virtual void read (Tango::DeviceImpl*, Tango::Attribute& a)
  {
    //- YAT4TANGO_TRACE("DynamicScalarAttribute::read");
    if (this->dai_.rcb.is_empty())
    {
      THROW_DEVFAILED("DEVICE_ERROR",
                      "this dynamic scalar attribute has no associated <read callback> [this is a programming error]",
                      "yat4tango::DynamicAttribute::read");
    }
    DynamicAttributeReadCallbackData cbd;
    cbd.dev = this->dai_.dev;
    cbd.dya = this;
    cbd.tga = &a;
    this->dai_.rcb(cbd);
  }
  //---------------------------------------------------------------------------
  //- called by Tango on client <read> request
  //---------------------------------------------------------------------------
  virtual void write (Tango::DeviceImpl*, Tango::WAttribute& a)
  {
    //- YAT4TANGO_TRACE("DynamicScalarAttribute::write");

    Tango::AttrWriteType attr_type = this->get_writable();
    if (attr_type != Tango::READ_WRITE && attr_type != Tango::WRITE)
    {
      THROW_DEVFAILED("DEVICE_ERROR",
                      "this dynamic scalar attribute is not writable!",
                      "yat4tango::DynamicScalarAttribute::write");
    }
    if (this->dai_.wcb.is_empty())
    {
      THROW_DEVFAILED("DEVICE_ERROR",
                      "this writable dynamic scalar attribute has no associated <write callback> [this is a programming error]",
                      "yat4tango::DynamicScalarAttribute::write");
    }
    DynamicAttributeWriteCallbackData cbd;
    cbd.dev = this->dai_.dev;
    cbd.dya = this;
    cbd.tga = &a;
    this->dai_.wcb(cbd);
  }
  //---------------------------------------------------------------------------
  //- is_allowed --------------------------------------------------------------
  //---------------------------------------------------------------------------
	virtual bool is_allowed (Tango::DeviceImpl *, Tango::AttReqType)
	{
		return true;
	}
};

// ============================================================================
// template class DynamicSpectrumAttribute
// ============================================================================
template <typename T>
class DynamicSpectrumAttribute : public Tango::SpectrumAttr, public DynamicAttribute
{
public:
  //---------------------------------------------------------------------------
  //- ctor
  //---------------------------------------------------------------------------
  DynamicSpectrumAttribute (const DynamicAttributeInfo& i)
    : Tango::SpectrumAttr (i.lan.c_str(),
                           i.tai.data_type,
                           i.tai.writable,
                           i.tai.max_dim_x,
                           i.tai.disp_level),
      DynamicAttribute (i)
  {
    //- YAT4TANGO_TRACE("DynamicSpectrumAttribute::DynamicSpectrumAttribute");
    set_user_defined_properties(*this, i);
  }
  //---------------------------------------------------------------------------
  //- dtor
  //---------------------------------------------------------------------------
  ~DynamicSpectrumAttribute()
  {
    //- YAT4TANGO_TRACE("DynamicSpectrumAttribute::~DynamicSpectrumAttribute");
  }
  //---------------------------------------------------------------------------
  //- returns the underlying Tango::Attr
  //---------------------------------------------------------------------------
  virtual Tango::Attr * get_tango_attribute ()
  {
    return this;
  }
  //---------------------------------------------------------------------------
  //- returns the underlying Tango data type of this DynamicAttribute
  //---------------------------------------------------------------------------
  virtual int get_tango_data_type () const
  {
    return TangoTraits<T>::type_id;
  }
  //---------------------------------------------------------------------------
  //- called by Tango on client <read> request
  //---------------------------------------------------------------------------
  virtual void read (Tango::DeviceImpl*, Tango::Attribute& a)
  {
    //- YAT4TANGO_TRACE("DynamicSpectrumAttribute::read");
    if (this->dai_.rcb.is_empty())
    {
      THROW_DEVFAILED("DEVICE_ERROR",
                      "this dynamic spectrum attribute has no associated <read callback> [this is a programming error]",
                      "yat4tango::DynamicSpectrumAttribute::read");
    }
    DynamicAttributeReadCallbackData cbd;
    cbd.dev = this->dai_.dev;
    cbd.dya = this;
    cbd.tga = &a;
    this->dai_.rcb(cbd);
  }
  //---------------------------------------------------------------------------
  //- called by Tango on client <read> request
  //---------------------------------------------------------------------------
  virtual void write (Tango::DeviceImpl*, Tango::WAttribute& a)
  {
    //- YAT4TANGO_TRACE("DynamicSpectrumAttribute::write");
    Tango::AttrWriteType attr_type = this->get_writable();
    if (attr_type != Tango::READ_WRITE && attr_type != Tango::WRITE)
    {
      THROW_DEVFAILED("DEVICE_ERROR",
                      "this dynamic spectrum attribute is not writable!",
                      "yat4tango::DynamicSpectrumAttribute::write");
    }
    if (this->dai_.wcb.is_empty())
    {
      THROW_DEVFAILED("DEVICE_ERROR",
                      "this writable dynamic spectrum attribute has no associated <write callback> [this is a programming error]",
                      "yat4tango::DynamicSpectrumAttribute::write");
    }
    DynamicAttributeWriteCallbackData cbd;
    cbd.dev = this->dai_.dev;
    cbd.dya = this;
    cbd.tga = &a;
    this->dai_.wcb(cbd);
  }
  //---------------------------------------------------------------------------
  //- is_allowed
  //---------------------------------------------------------------------------
	virtual bool is_allowed (Tango::DeviceImpl *, Tango::AttReqType)
	{
		return true;
	}
};

// ============================================================================
// template class DynamicImageAttribute
// ============================================================================
template <typename T>
class DynamicImageAttribute : public Tango::ImageAttr, public DynamicAttribute
{
public:
  //---------------------------------------------------------------------------
  //- ctor
  //---------------------------------------------------------------------------
  DynamicImageAttribute (const DynamicAttributeInfo& i)
    : Tango::ImageAttr (i.lan.c_str(),
                        i.tai.data_type,
                        i.tai.writable,
                        i.tai.max_dim_x,
                        i.tai.max_dim_y,
                        i.tai.disp_level),
      DynamicAttribute (i)
  {
    //- YAT4TANGO_TRACE("DynamicImageAttribute::DynamicImageAttribute");
    set_user_defined_properties(*this, i);
  }
  //---------------------------------------------------------------------------
  //- dtor
  //---------------------------------------------------------------------------
  ~DynamicImageAttribute ()
  {
    //- no log here, crash otherwise!
  }
  //---------------------------------------------------------------------------
  //- returns the underlying Tango::Attr
  //---------------------------------------------------------------------------
  virtual Tango::Attr * get_tango_attribute ()
  {
    return this;
  }
  //---------------------------------------------------------------------------
  //- returns the underlying Tango data type of this DynamicAttribute
  //---------------------------------------------------------------------------
  virtual int get_tango_data_type () const
  {
    return TangoTraits<T>::type_id;
  }
  //---------------------------------------------------------------------------
  //- called by Tango on client <read> request
  //---------------------------------------------------------------------------
  virtual void read (Tango::DeviceImpl*, Tango::Attribute& a)
  {
    //- YAT4TANGO_TRACE("DynamicImageAttribute::read");
    if (this->dai_.rcb.is_empty())
    {
      THROW_DEVFAILED("DEVICE_ERROR",
                      "this dynamic image attribute has no associated <read callback> [this is a programming error]",
                      "yat4tango::DynamicImageAttribute::read");
    }
    DynamicAttributeReadCallbackData cbd;
    cbd.dev = this->dai_.dev;
    cbd.dya = this;
    cbd.tga = &a;
    this->dai_.rcb(cbd);
  }
  //---------------------------------------------------------------------------
  //- called by Tango on client <read> request
  //---------------------------------------------------------------------------
  virtual void write (Tango::DeviceImpl*, Tango::WAttribute& a)
  {
    //- YAT4TANGO_TRACE("DynamicImageAttribute::write");
    Tango::AttrWriteType attr_type = this->get_writable();
    if (attr_type != Tango::READ_WRITE && attr_type != Tango::WRITE)
    {
      THROW_DEVFAILED("DEVICE_ERROR",
                      "this dynamic image attribute is not writable!",
                      "yat4tango::DynamicImageAttribute::write");
    }
    if (this->dai_.wcb.is_empty())
    {
      THROW_DEVFAILED("DEVICE_ERROR",
                      "this writable dynamic image attribute has no associated <write callback> [this is a programming error]",
                      "yat4tango::DynamicImageAttribute::write");
    }
    DynamicAttributeWriteCallbackData cbd;
    cbd.dev = this->dai_.dev;
    cbd.dya = this;
    cbd.tga = &a;
    this->dai_.wcb(cbd);
  }
  //---------------------------------------------------------------------------
  //- is_allowed
  //---------------------------------------------------------------------------
	virtual bool is_allowed (Tango::DeviceImpl *, Tango::AttReqType)
	{
		return true;
	}
};

// ============================================================================
// DynamicAttributeFactory::create_attribute
// ============================================================================
DynamicAttribute * DynamicAttributeFactory::create_attribute (const yat4tango::DynamicAttributeInfo& i)
{
  //- no dynamic attr. named state or status
  std::string a(i.lan);
  std::transform(a.begin(), a.end(), a.begin(), (int(*)(int))std::tolower);

  if ( a == "state" || a == "status" )
  {
    THROW_DEVFAILED("DEVICE_ERROR",
                    "can't create a dynamic attribute named <State> or <Status>",
                    "yat4tango::DynamicAttributeFactory::create_attribute");
  }

  //- instanciate
  DynamicAttribute * da = 0;

  try
  {
	  switch ( i.tai.data_format )
	  {
      //- SCALAR --------------------------------------------------------------
	    case Tango::SCALAR:
		  {
			  switch ( i.tai.data_type )
			  {
			    case Tango::DEV_BOOLEAN:
                    da = new DynamicScalarAttribute<Tango::DevBoolean>(i);
                    break;
  			    case Tango::DEV_UCHAR:
                    da = new DynamicScalarAttribute<Tango::DevUChar>(i);
                    break;
#if (TANGO_VERSION_MAJOR >= 9)
                case Tango::DEV_ENUM:
#endif
			    case Tango::DEV_SHORT:
                    da = new DynamicScalarAttribute<Tango::DevShort>(i);
                    break;
			    case Tango::DEV_USHORT:
                    da = new DynamicScalarAttribute<Tango::DevUShort>(i);
                    break;
			    case Tango::DEV_LONG:
                    da = new DynamicScalarAttribute<Tango::DevLong>(i);
                    break;
			    case Tango::DEV_ULONG:
                    da = new DynamicScalarAttribute<Tango::DevULong>(i);
                    break;
			    case Tango::DEV_LONG64:
                    da = new DynamicScalarAttribute<Tango::DevLong64>(i);
                    break;
			    case Tango::DEV_ULONG64:
                    da = new DynamicScalarAttribute<Tango::DevULong64>(i);
                    break;
  			    case Tango::DEV_FLOAT:
                    da = new DynamicScalarAttribute<Tango::DevFloat>(i);
                    break;
			    case Tango::DEV_DOUBLE:
                    da = new DynamicScalarAttribute<Tango::DevDouble>(i);
                    break;
			    case Tango::DEV_STRING:
                    da = new DynamicScalarAttribute<Tango::DevString>(i);
                    break;
			    case Tango::DEV_STATE:
                    da = new DynamicScalarAttribute<Tango::DevState>(i);
                    break;
			    case Tango::DEV_ENCODED:
                    da = new DynamicScalarAttribute<Tango::DevEncoded>(i);
                    break;
                default:
                    THROW_DEVFAILED("UNSUPPORTED_FEATURE",
                                    "the requested dynamic attribute data type is not supported",
                                    "DynamicAttributeFactory::create_attribute");
                 break;
			  }
		  }
		  break;
        //- SPECTRUM ------------------------------------------------------------
	    case Tango::SPECTRUM:
		  {
		      switch ( i.tai.data_type )
		      {
		        case Tango::DEV_BOOLEAN:
                    da = new DynamicSpectrumAttribute<Tango::DevBoolean>(i);
                    break;
		        case Tango::DEV_UCHAR:
                    da = new DynamicSpectrumAttribute<Tango::DevUChar>(i);
                    break;
		        case Tango::DEV_SHORT:
                    da = new DynamicSpectrumAttribute<Tango::DevShort>(i);
                    break;
		        case Tango::DEV_USHORT:
                    da = new DynamicSpectrumAttribute<Tango::DevUShort>(i);
                    break;
		        case Tango::DEV_LONG:
                    da = new DynamicSpectrumAttribute<Tango::DevLong>(i);
                    break;
		        case Tango::DEV_ULONG:
                    da = new DynamicSpectrumAttribute<Tango::DevULong>(i);
                    break;
		        case Tango::DEV_LONG64:
                    da = new DynamicSpectrumAttribute<Tango::DevLong64>(i);
                    break;
		        case Tango::DEV_ULONG64:
                    da = new DynamicSpectrumAttribute<Tango::DevULong64>(i);
                    break;
		        case Tango::DEV_FLOAT:
                    da = new DynamicSpectrumAttribute<Tango::DevFloat>(i);
                    break;
		        case Tango::DEV_DOUBLE:
                    da = new DynamicSpectrumAttribute<Tango::DevDouble>(i);
                    break;
		        case Tango::DEV_STRING:
                    da = new DynamicSpectrumAttribute<Tango::DevString>(i);
                    break;
		        case Tango::DEV_STATE:
                    da = new DynamicSpectrumAttribute<Tango::DevState>(i);
                    break;
		        case Tango::DEV_ENCODED:
                    da = new DynamicSpectrumAttribute<Tango::DevEncoded>(i);
                    break;
                default:
                    THROW_DEVFAILED("UNSUPPORTED_FEATURE",
                                    "the requested dynamic attribute data type is not supported",
                                    "DynamicAttributeFactory::create_attribute");
                 break;
		      }
		  }
		  break;
          //- IMAGE ---------------------------------------------------------------
	      case Tango::IMAGE:
		  {
			  switch ( i.tai.data_type )
			  {
			    case Tango::DEV_BOOLEAN:
                    da = new DynamicImageAttribute<Tango::DevBoolean>(i);
                    break;
  			    case Tango::DEV_UCHAR:
                    da = new DynamicImageAttribute<Tango::DevUChar>(i);
                    break;
			    case Tango::DEV_SHORT:
                    da = new DynamicImageAttribute<Tango::DevShort>(i);
                    break;
			    case Tango::DEV_USHORT:
                    da = new DynamicImageAttribute<Tango::DevUShort>(i);
                    break;
			    case Tango::DEV_LONG:
                    da = new DynamicImageAttribute<Tango::DevLong>(i);
                    break;
			    case Tango::DEV_ULONG:
                    da = new DynamicImageAttribute<Tango::DevULong>(i);
                    break;
			    case Tango::DEV_LONG64:
                    da = new DynamicImageAttribute<Tango::DevLong64>(i);
                    break;
			    case Tango::DEV_ULONG64:
                    da = new DynamicImageAttribute<Tango::DevULong64>(i);
                    break;
  			  case Tango::DEV_FLOAT:
                    da = new DynamicImageAttribute<Tango::DevFloat>(i);
                    break;
			    case Tango::DEV_DOUBLE:
                    da = new DynamicImageAttribute<Tango::DevDouble>(i);
                    break;
			    case Tango::DEV_STRING:
                    da = new DynamicImageAttribute<Tango::DevString>(i);
                    break;
			    case Tango::DEV_STATE:
                    da = new DynamicImageAttribute<Tango::DevState>(i);
                    break;
			    case Tango::DEV_ENCODED:
                    da = new DynamicImageAttribute<Tango::DevEncoded>(i);
                    break;
                default:
                    THROW_DEVFAILED("UNSUPPORTED_FEATURE",
                                    "the requested dynamic attribute data type is not supported",
                                    "DynamicAttributeFactory::create_attribute");
                    break;
			  }
		  }
		  break;
          default:
            THROW_DEVFAILED("UNSUPPORTED_FEATURE",
                            "the request dynamic attribute format is not yet supported",
                            "DynamicAttributeFactory::create_attribute");
          break;
	  }
  }
  catch (Tango::DevFailed& df)
  {
    delete da;
    RETHROW_DEVFAILED(df,
                      "DEVICE_ERROR",
                      "DynamicAttribute allocation failed [Tango exception caught]",
                      "DynamicAttributeFactory::create_attribute");
  }
  catch (...)
  {
    delete da;
    THROW_DEVFAILED("INTERNAL_ERROR",
                    "DynamicAttribute allocation failed [unknown exception caught]",
                    "DynamicAttributeFactory::create_attribute");
  }

  if (! da)
  {
    THROW_DEVFAILED("MEMORY_ERROR",
                    "DynamicAttribute allocation failed",
                    "DynamicAttributeFactory::create_attribute");
  }

	return da;
}

// ============================================================================
// DynamicAttributeFactory::create_attribute
// ============================================================================
DynamicAttribute * DynamicAttributeFactory::create_attribute (const yat4tango::ForwardedAttributeInfo& i)
{
  //- extract dev name & attr name from url
  yat::StringTokenizer st(i.url, "/");
  size_t nt = st.count_tokens();
  if ( nt > 1  && ( nt < 4 || nt > 5 ) )
  {
    std::ostringstream oss;
    oss << "invalid fully qualified attribute name <"
        << i.url
        << "> - should be something like [db-host:db-port/]tango/dev/name/attr-name";
    THROW_DEVFAILED("API_ERROR",
                    oss.str().c_str(),
                    "DynamicAttributeFactory::create_attribute");
  }

  //- dev name
  std::string dev;
  //- 'remote' attr name
  std::string remote_attr_name;

  if ( nt > 1 )
  {
    //- fully qualified attribute name
    if ( nt == 5 )
    {
      dev  = st.next_token();
      dev += "/";
    }
    dev += st.next_token();
    dev += "/";
    dev += st.next_token();
    dev += "/";
    dev += st.next_token();
    remote_attr_name = st.next_token();
  }
  else
  {
    //- alias ?
    try
    {
      Tango::AttributeProxy ap(i.url.c_str());
      dev = ap.get_device_proxy()->dev_name();
      remote_attr_name = ap.name();
    }
    catch (Tango::DevFailed& e)
    {
      std::ostringstream oss;
      oss << "invalid/unknown attribute alias - '"
          << i.url
          << "' is invalid";
      RETHROW_DEVFAILED(e,
                        "API_ERROR",
                        oss.str().c_str(),
                        "DynamicAttributeFactory::create_attribute");
    }
    catch (...)
    {
      std::ostringstream oss;
      oss << "unknown exception caught while trying to forward attribute '"
          << i.url
          << "'";
      THROW_DEVFAILED("API_ERROR",
                      oss.str().c_str(),
                      "DynamicAttributeFactory::create_attribute");
    }
  }

  //- 'local' attribute name (keep original name?)
  std::string local_attr_name;
  if ( i.lan != yat4tango::KeepOriginalAttributeName )
    local_attr_name = i.lan;
  else
    local_attr_name = remote_attr_name;

  //- instanciate the device proxy
  Tango::DeviceProxy * dp = 0;
  try
  {
    dp = new Tango::DeviceProxy(dev);
  }
  catch (Tango::DevFailed& df)
  {
    std::ostringstream oss;
    oss << "failed to instanciate a DeviceProxy for <"
        << dev
        << "> [hint: is this device registered into the Tango db?]";
    RETHROW_DEVFAILED(df,
                      "API_ERROR",
                      oss.str().c_str(),
                      "DynamicAttributeFactory::create_attribute");
  }
  catch (...)
  {
    std::ostringstream oss;
    oss << "failed to instanciate a DeviceProxy for <"
        << dev
        << "> [unknown exception caught]";
    THROW_DEVFAILED("API_ERROR",
                    oss.str().c_str(),
                    "DynamicAttributeFactory::create_attribute");
  }

  //- get attribute info
  Tango::AttributeInfoEx remote_attr_info;
  try
  {
    remote_attr_info = dp->get_attribute_config(remote_attr_name);
  }
  catch (Tango::DevFailed& df)
  {
    delete dp;
    std::ostringstream oss;
    oss << "failed to obtain attribute info for <"
        << dev
        << "/"
        << remote_attr_name
        << "> [hint: is it an attribute of the specified device?]";
    RETHROW_DEVFAILED(df,
                      "API_ERROR",
                      oss.str().c_str(),
                      "DynamicAttributeFactory::create_attribute");
  }
  catch (...)
  {
    delete dp;
    std::ostringstream oss;
    oss << "failed to obtain attribute info for <"
        << dev
        << "/"
        << remote_attr_name
        << "> [unknown exception caught]";
    THROW_DEVFAILED("API_ERROR",
                    oss.str().c_str(),
                    "DynamicAttributeFactory::create_attribute");
  }

  //- is the attribute access restricted to read-only?
  if ( i.rdo )
  {
    remote_attr_info.writable = Tango::READ;
  }

  //- describe & instanciate the forwarded attribute
  yat4tango::DynamicAttributeInfo dai;
  dai.dev = i.dev;
  dai.tai = remote_attr_info;
  dai.lan = local_attr_name;
  DynamicAttribute * da = this->create_attribute(dai);

  //- associates the forwarded attribute with its device proxy (callee gets ownership of dp)
  da->link_to_proxy(dp);

  return da;
}

} //- namespace
