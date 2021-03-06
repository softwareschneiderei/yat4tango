//----------------------------------------------------------------------------
// Copyright (c) 2004-2015 Synchrotron SOLEIL
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Lesser Public License v3
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/lgpl.html
//----------------------------------------------------------------------------
/*----- PROTECTED REGION ID(DynamicInterface.cpp) ENABLED START -----*/
static const char *RcsId = "$Id:  $";
//=============================================================================
//
// file :        DynamicInterface.cpp
//
// description : C++ source for the DynamicInterface and its commands.
//               The class is derived from Device. It represents the
//               CORBA servant object which will be accessed from the
//               network. All commands which can be executed on the
//               DynamicInterface are implemented in this file.
//
// project :     DynamicInterface.
//
// $Author:  N.Leclercq - SOLEIL
//
// $Revision:  $
// $Date:  $
//
// SVN only:
// $HeadURL:  $
//
// CVS only:
// $Source:  $
// $Log:  $
//
//=============================================================================
//                This file is generated by POGO
//        (Program Obviously used to Generate tango Object)
//=============================================================================


#include <DynamicInterface.h>
#include <DynamicInterfaceClass.h>
#include <yat/utils/StringTokenizer.h>
#include <yat4tango/InnerAppender.h>
#include <yat4tango/ExceptionHelper.h>

/*----- PROTECTED REGION END -----*/


/**
 *	DynamicInterface class description:
 *	Forwards the attributes specified by the ForwardedAttributes property.
 *	Each forwarded attribute becomes a local dynamic attribute of the DynamicInterface device.
 *	ForwardedAttributes property is an array of strings (one entry per attribute to forward).
 *	Each entry should at least contains the fully qualified name of the attribute to be forwarded.
 *	An optional 'local attribute name' can also be specified in case you need/want to change the name under which the attribute is forwarded.
 *	Each ForwardedAttributes entry must have the following syntax: [tdb-host:tdb-port/]the/dev/name/attr-name[::attr-local-name].
 *
 */

//================================================================
//
//  The following table gives the correspondence
//  between command and method names.
//
//  Command name  |  Method name
//----------------------------------------------------------------
//  State         |  Inherited (no method)
//  Status        |  Inherited (no method)
//================================================================

namespace DynamicInterface_ns
{
	/*----- PROTECTED REGION ID(DynamicInterface::namespace_starting) ENABLED START -----*/

	//	static initializations

	/*----- PROTECTED REGION END -----*/	//	DynamicInterface::namespace_starting

//--------------------------------------------------------
/**
 *	Method      : DynamicInterface::DynamicInterface()
 *	Description : Constructors for a Tango device
 *	              implementing the class DynamicInterface
 */
//--------------------------------------------------------
DynamicInterface::DynamicInterface(Tango::DeviceClass *cl, string &s)
 	: Tango::Device_4Impl(cl, s.c_str()), m_dim(this)
{
	/*----- PROTECTED REGION ID(DynamicInterface::constructor_1) ENABLED START -----*/

	init_device();

	/*----- PROTECTED REGION END -----*/	//	DynamicInterface::constructor_1
}
//--------------------------------------------------------
DynamicInterface::DynamicInterface(Tango::DeviceClass *cl, const char *s)
 	: Tango::Device_4Impl(cl, s), m_dim(this)
{
	/*----- PROTECTED REGION ID(DynamicInterface::constructor_2) ENABLED START -----*/

	init_device();

	/*----- PROTECTED REGION END -----*/	//	DynamicInterface::constructor_2
}
//--------------------------------------------------------
DynamicInterface::DynamicInterface(Tango::DeviceClass *cl, const char *s, const char *d)
 	: Tango::Device_4Impl(cl, s, d), m_dim(this)
{
	/*----- PROTECTED REGION ID(DynamicInterface::constructor_3) ENABLED START -----*/

	init_device();

	/*----- PROTECTED REGION END -----*/	//	DynamicInterface::constructor_3
}


//--------------------------------------------------------
/**
 *	Method      : DynamicInterface::delete_device()()
 *	Description : will be called at device destruction or at init command
 */
//--------------------------------------------------------
void DynamicInterface::delete_device()
{
	/*----- PROTECTED REGION ID(DynamicInterface::delete_device) ENABLED START -----*/

  //- remove any dynamic attr or command
  this->m_dim.remove();

  //- remove the inner appender
  yat4tango::InnerAppender::release(this);

	/*----- PROTECTED REGION END -----*/	//	DynamicInterface::delete_devic
}


//--------------------------------------------------------
/**
 *	Method      : DynamicInterface::init_device()
 *	Description : //	will be called at device initialization.
 */
//--------------------------------------------------------
void DynamicInterface::init_device()
{
	/*----- PROTECTED REGION ID(DynamicInterface::init_device_before) ENABLED START -----*/

   //- initialize the inner appender (first thing to do)
  try
  {
    yat4tango::InnerAppender::initialize(this, 512);
  }
  catch ( Tango::DevFailed& df )
  {
    std::string err("DynamicInterface::init_device failed - could not instanciate the InnerAppender");
    ERROR_STREAM << err << std::endl;
    ERROR_STREAM << df << std::endl;
    this->set_state(Tango::FAULT);
    this->set_status(err);
    return;
  }

  //- trace/profile this method
  YAT4TANGO_TRACE("DynamicInterface::init_device");


	/*----- PROTECTED REGION END -----*/	//	DynamicInterface::init_device_before

	//	Get the device properties (if any) from database
	get_device_property();


	/*----- PROTECTED REGION ID(DynamicInterface::init_device) ENABLED START -----*/

  //- add some dynamic attributes
  try
  {
    for (size_t i = 0; i < 3; ++i)
    {
      yat4tango::DynamicAttributeInfo dai;
      dai.dev = this;
      switch (i)
      {
        case 0:
          //- initialize the associated data (here its a simple short)
          attr1_val = 10;
          //- specify the dyn. attr.  name
          dai.tai.name = "dynAttr1";
          //- associate the dyn. attr. with its data (see read_callback for usage)
          dai.set_user_data(&attr1_val);
          break;
        case 1:
          //- initialize the associated data (here its a simple short)
          attr2_val = 20;
          //- specify the dyn. attr.  name
          dai.tai.name = "dynAttr2";
          //- associate the dyn. attr. with its data (see read_callback for usage)
          dai.set_user_data(&attr2_val);
          break;
        case 2:
          //- initialize the associated data (here its a simple short)
          attr3_val = 30;
          //- specify the dyn. attr.  name
          dai.tai.name = "dynAttr3";
          //- associate the dyn. attr. with its data (see read_callback for usage)
          dai.set_user_data(&attr3_val);
          break;
        default:
          break;
      }
      //- describe the dynamic attr we want...
      dai.tai.data_type   = Tango::DEV_SHORT;
      dai.tai.data_format = Tango::SCALAR;
      dai.tai.writable    = Tango::READ_WRITE;
      dai.tai.disp_level  = Tango::OPERATOR;
      //- cleanup tango db option: cleanup tango db when removing this dyn. attr. (i.e. erase its properties fom db)
      dai.cdb = true;
      //- instanciate the read callback (called when the dyn. attr. is read)
      dai.rcb = yat4tango::DynamicAttributeReadCallback::instanciate(*this,
                                                                     &DynamicInterface::read_callback);
      //- instanciate the write callback (called when the dyn. attr. is written)
      dai.wcb = yat4tango::DynamicAttributeWriteCallback::instanciate(*this,
                                                                      &DynamicInterface::write_callback);
      //- log
      INFO_STREAM << "adding dyn. attribute "
                  << dai.tai.name
                  << std::endl;
      //- add the dyn. attr. to the device
      this->m_dim.dynamic_attributes_manager().add_attribute(dai);
    }
  }
  catch (Tango::DevFailed& df)
  {
    std::string err("failed to instanciate dynamic attributes  - Tango exception caught - see log attribute for details");
    ERROR_STREAM << err << std::endl;
    ERROR_STREAM << df << std::endl;
    this->set_state(Tango::FAULT);
    this->set_status(err);
    return;
  }
  catch (...)
  {
    this->set_state(Tango::FAULT);
    this->set_status("failed to instanciate dynamic attributes - unknown exception caught");
    return;
  }

  //- add some dynamic commands
  try
  {
    for (size_t i = 0; i < 22; ++i)
    {
      //- populate the command info data struct
      yat4tango::DynamicCommandInfo dci;
      //- the device to hosting the dynamic interface we are building
      dci.dev = this;
      //- in this example, user data is used to identify the command
      dci.set_user_data(i);
      //- specify the command name and the in & out arguments data type
      switch (i)
      {
        case 0:
          dci.tci.cmd_name = "DynCmdVoid";
          dci.tci.in_type  = Tango::DEV_VOID;
          dci.tci.out_type = Tango::DEV_VOID;
          break;
        case 1:
          dci.tci.cmd_name = "DynCmdBoolean";
          dci.tci.in_type  = Tango::DEV_BOOLEAN;
          dci.tci.out_type = Tango::DEV_BOOLEAN;
          break;
        case 2:
          dci.tci.cmd_name = "DynCmdShort";
          dci.tci.in_type  = Tango::DEV_SHORT;
          dci.tci.out_type = Tango::DEV_SHORT;
          break;
        case 3:
          dci.tci.cmd_name = "DynCmdUShort";
          dci.tci.in_type  = Tango::DEV_USHORT;
          dci.tci.out_type = Tango::DEV_USHORT;
          break;
        case 4:
          dci.tci.cmd_name = "DynCmdLong";
          dci.tci.in_type  = Tango::DEV_LONG;
          dci.tci.out_type = Tango::DEV_LONG;
          break;
        case 5:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdULong";
          dci.tci.in_type  = Tango::DEV_ULONG;
          dci.tci.out_type = Tango::DEV_ULONG;
          break;
        case 6:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdLong64";
          dci.tci.in_type  = Tango::DEV_LONG64;
          dci.tci.out_type = Tango::DEV_LONG64;
          break;
        case 7:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdULong64";
          dci.tci.in_type  = Tango::DEV_ULONG64;
          dci.tci.out_type = Tango::DEV_ULONG64;
          break;
        case 8:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdFloat";
          dci.tci.in_type  = Tango::DEV_FLOAT;
          dci.tci.out_type = Tango::DEV_FLOAT;
          break;
        case 9:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdDouble";
          dci.tci.in_type  = Tango::DEV_DOUBLE;
          dci.tci.out_type = Tango::DEV_DOUBLE;
          break;
        case 10:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdLongStringArray";
          dci.tci.in_type  = Tango::DEVVAR_LONGSTRINGARRAY;
          dci.tci.out_type = Tango::DEVVAR_LONGSTRINGARRAY;
          break;
        case 11:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdDoubleStringArray";
          dci.tci.in_type  = Tango::DEVVAR_DOUBLESTRINGARRAY;
          dci.tci.out_type = Tango::DEVVAR_DOUBLESTRINGARRAY;
          break;
        case 12:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdStringArray";
          dci.tci.in_type  = Tango::DEVVAR_STRINGARRAY;
          dci.tci.out_type = Tango::DEVVAR_STRINGARRAY;
          break;
        case 13:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdCharArray";
          dci.tci.in_type  = Tango::DEVVAR_CHARARRAY;
          dci.tci.out_type = Tango::DEVVAR_CHARARRAY;
          break;
        case 14:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdShortArray";
          dci.tci.in_type  = Tango::DEVVAR_SHORTARRAY;
          dci.tci.out_type = Tango::DEVVAR_SHORTARRAY;
          break;
        case 15:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdUShortArray";
          dci.tci.in_type  = Tango::DEVVAR_USHORTARRAY;
          dci.tci.out_type = Tango::DEVVAR_USHORTARRAY;
          break;
        case 16:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdLongArray";
          dci.tci.in_type  = Tango::DEVVAR_LONGARRAY;
          dci.tci.out_type = Tango::DEVVAR_LONGARRAY;
          break;
        case 17:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdULongArray";
          dci.tci.in_type  = Tango::DEVVAR_ULONGARRAY;
          dci.tci.out_type = Tango::DEVVAR_ULONGARRAY;
          break;
        case 18:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdLong64Array";
          dci.tci.in_type  = Tango::DEVVAR_LONG64ARRAY;
          dci.tci.out_type = Tango::DEVVAR_LONG64ARRAY;
          break;
        case 19:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdULong64Array";
          dci.tci.in_type  = Tango::DEVVAR_ULONG64ARRAY;
          dci.tci.out_type = Tango::DEVVAR_ULONG64ARRAY;
          break;
        case 20:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdFloatArray";
          dci.tci.in_type  = Tango::DEVVAR_FLOATARRAY;
          dci.tci.out_type = Tango::DEVVAR_FLOATARRAY;
          break;
        case 21:
          //- specify the dyn. attr.  name
          dci.tci.cmd_name = "DynCmdDoubleArray";
          dci.tci.in_type  = Tango::DEVVAR_DOUBLEARRAY;
          dci.tci.out_type = Tango::DEVVAR_DOUBLEARRAY;
          break;
        default:
          break;
      }
      //- operator command
      dci.tci.disp_level = Tango::OPERATOR;
      //- instanciate the read callback (called when the dyn. attr. is read)
      dci.ecb = yat4tango::DynamicCommandExecuteCallback::instanciate(*this,
                                                                      &DynamicInterface::execute_callback);
      //- log
      INFO_STREAM << "adding dyn. command "
                  << dci.tci.cmd_name
                  << std::endl;
      //- add the dyn. attr. to the device
      this->m_dim.dynamic_commands_manager().add_command(dci);
    }
  }
  catch (Tango::DevFailed& df)
  {
    std::string err("failed to instanciate dynamic commands - Tango exception caught - see log attribute for details");
    ERROR_STREAM << err << std::endl;
    ERROR_STREAM << df << std::endl;
    this->set_state(Tango::FAULT);
    this->set_status(err);
    return;
  }
  catch (...)
  {
    this->set_state(Tango::FAULT);
    this->set_status("failed to instanciate dynamic commands - unknown exception caught");
    return;
  }

  this->set_state(Tango::RUNNING);

  DEBUG_STREAM << "device successfully initialized" << std::endl;

	/*----- PROTECTED REGION END -----*/	//	DynamicInterface::init_device
}

//----------------------------------------------------------------------------------------
// GENERIC CMD EXECUTION HELPER (IN THIS EXAMPLE, ALL CMD RETURN THEIR INPUT ARG)
//----------------------------------------------------------------------------------------
template <typename T>
void execute_scalar_cmd (yat4tango::DynamicCommandExecuteCallbackData& cbd)
{
  T cmd_data;

  //- extract argin
  cbd.dyc->get_argin(cmd_data);

  //- insert argout
  cbd.dyc->set_argout(cmd_data);
}

//----------------------------------------------------------------------------------------
// GENERIC CMD EXECUTION HELPER (IN THIS EXAMPLE, ALL CMD RETURN THEIR INPUT ARG)
//----------------------------------------------------------------------------------------
template <typename T>
void execute_array_cmd (yat4tango::DynamicCommandExecuteCallbackData& cbd)
{
  //- extract argin
  const T * argin;
  cbd.dyc->get_argin(argin);

  //- allocate argout
  T * argout = new T();
  if ( ! argout )
  {
    THROW_DEVFAILED("MEMORY_ERROR",
                    "failed to allocate output argument array",
                    "DynamicInterface::execute_array_cmd");
  }

  //- copy argin to argout (that's what each command does in this example)
  *argout = *argin;

  //- insert argout
  cbd.dyc->set_argout(argout);
}

//----------------------------------------------------------------------------------------
// DynamicInterface::execute_callback
//----------------------------------------------------------------------------------------
void DynamicInterface::execute_callback (yat4tango::DynamicCommandExecuteCallbackData& cbd)
{
  //- be sure the pointer to the dyn. attr. is valid
  if ( ! cbd.dyc )
  {
    THROW_DEVFAILED("INTERNAL_ERROR",
                    "unexpected NULL pointer to dynamic command",
                    "DynamicInterface::execute_callback");
  }

  //- log
  DEBUG_STREAM << "handling execute request for dynamic command: "
               << cbd.dyc->get_name()
               << std::endl;

  //- in this example, user data is used to identify the command
  size_t cmd_id;
  cbd.dyc->get_user_data(cmd_id);

  switch ( cmd_id )
  {
    case 0:
      //- DynCmdVoid
      break;
    case 1:
      //- DynCmdBoolean
      execute_scalar_cmd<Tango::DevBoolean>(cbd);
      break;
    case 2:
      //- DynCmdShort
      execute_scalar_cmd<Tango::DevShort>(cbd);
      break;
    case 3:
      //- DynCmdUShort
      execute_scalar_cmd<Tango::DevUShort>(cbd);
      break;
    case 4:
      //- DynCmdLong
      execute_scalar_cmd<Tango::DevLong>(cbd);
      break;
    case 5:
      //- DynCmdULong
      execute_scalar_cmd<Tango::DevULong>(cbd);
      break;
    case 6:
      //- DynCmdLong64
      execute_scalar_cmd<Tango::DevLong64>(cbd);
      break;
    case 7:
      //- DynCmdULong64
      execute_scalar_cmd<Tango::DevULong64>(cbd);
      break;
    case 8:
      //- DynCmdFloat
      execute_scalar_cmd<Tango::DevFloat>(cbd);
      break;
    case 9:
      //- DynCmdDouble
      execute_scalar_cmd<Tango::DevDouble>(cbd);
      break;
    case 13:
      //- DynCmdCharArray
      execute_array_cmd<Tango::DevVarCharArray>(cbd);
      break;
    case 14:
      //- DynCmdShortArray
      execute_array_cmd<Tango::DevVarShortArray>(cbd);
      break;
    case 15:
      //- DynCmdUShortArray
      execute_array_cmd<Tango::DevVarUShortArray>(cbd);
      break;
    case 16:
      //- DynCmdLongArray
      execute_array_cmd<Tango::DevVarLongArray>(cbd);
      break;
    case 17:
      //- DynCmdULongArray
      execute_array_cmd<Tango::DevVarULongArray>(cbd);
      break;
    case 18:
      //- DynCmdLong64Array
      execute_array_cmd<Tango::DevVarLong64Array>(cbd);
      break;
    case 19: //- DynCmdULong64Array
      execute_array_cmd<Tango::DevVarULong64Array>(cbd);
      break;
    case 20:
      //- DynCmdFloatArray
      execute_array_cmd<Tango::DevVarFloatArray>(cbd);
      break;
    case 21:
      //- DynCmdDoubleArray
      execute_array_cmd<Tango::DevVarDoubleArray>(cbd);
      break;
    case 10:
      //- DynCmdLongStringArray
      {
        //- get argin
        const Tango::DevVarLongStringArray * argin;
        cbd.dyc->get_argin(argin);

        //- instanciate argout
        Tango::DevVarLongStringArray * argout = new Tango::DevVarLongStringArray();
        if ( ! argout )
        {
          THROW_DEVFAILED("MEMORY_ERROR",
                          "failed to allocate output argument array",
                          "DynamicInterface::execute_cmd");
        }

        //- copy argin to argout (that's what each command does in this example)
        argout->lvalue = argin->lvalue;
        argout->svalue.length(argin->svalue.length());
        for ( size_t i = 0; i < argin->svalue.length(); i++ )
           argout->svalue[i] = CORBA::string_dup(argin->svalue[i]);

        //- insert argout
        cbd.dyc->set_argout(argout);
      }
      break;
    case 11:
      //- DynCmdDoubleStringArray
      {
        //- get argin
        const Tango::DevVarDoubleStringArray * argin;
        cbd.dyc->get_argin(argin);

        //- instanciate argout
        Tango::DevVarDoubleStringArray * argout = new Tango::DevVarDoubleStringArray();
        if ( ! argout )
        {
          THROW_DEVFAILED("MEMORY_ERROR",
                          "failed to allocate output argument array",
                          "DynamicInterface::execute_cmd");
        }

        //- copy argin to argout (that's what each command does in this example)
        argout->dvalue = argin->dvalue;
        argout->svalue.length(argin->svalue.length());
        for ( size_t i = 0; i < argin->svalue.length(); i++ )
           argout->svalue[i] = CORBA::string_dup(argin->svalue[i]);

        //- insert argout
        cbd.dyc->set_argout(argout);
      }
      break;
    case 12:
      //- DynCmdStringArray
      {
        //- get argin
        const Tango::DevVarStringArray * argin;
        cbd.dyc->get_argin(argin);

        //- instanciate argout
        Tango::DevVarStringArray * argout = new Tango::DevVarStringArray();
        if ( ! argout )
        {
          THROW_DEVFAILED("MEMORY_ERROR",
                          "failed to allocate output argument array",
                          "DynamicInterface::execute_cmd");
        }

        //- copy argin to argout (that's what each command does in this example)
        argout->length(argin->length());
        for ( size_t i = 0; i < argin->length(); i++ )
           (*argout)[i] = CORBA::string_dup((*argin)[i]);

        //- insert argout
        cbd.dyc->set_argout(argout);
      }
      break;
    default:
      break;
  }
}

//----------------------------------------------------------------------------------------
// DYN. ATTRS. READ CALLBACK
//----------------------------------------------------------------------------------------
void DynamicInterface::read_callback (yat4tango::DynamicAttributeReadCallbackData& cbd)
{
  //- be sure the pointer to the dyn. attr. is valid
  if ( ! cbd.dya )
  {
    THROW_DEVFAILED("INTERNAL_ERROR",
                    "unexpected NULL pointer to dynamic attribute",
                    "DynamicInterface::read_callback");
  }

  //- log
  DEBUG_STREAM << "handling read request for dynamic attribute: "
               << cbd.dya->get_name()
               << std::endl;

  //- get access to the data associated with the dynamic attribute
  //- we choose to retreive it using the dyn. attr. "user data"
  Tango::DevShort * val = cbd.dya->get_user_data<Tango::DevShort>();

  //- be sure the pointer is valid
  if ( ! val )
  {
    TangoSys_OMemStream s;
    s << "unexpected NULL pointer to dynamic attribute data for "
      << cbd.dya->get_name();
    THROW_DEVFAILED("INTERNAL_ERROR", s.str().c_str(), "DynamicInterface::read_callback");
  }

  //- change the attribute value (i.e. simulate some activity)
  (*val)++;

  //- set the attribute value
  cbd.tga->set_value(val);
}

//----------------------------------------------------------------------------------------
// DYN. ATTRS. WRITE CALLBACK
//----------------------------------------------------------------------------------------
void DynamicInterface::write_callback (yat4tango::DynamicAttributeWriteCallbackData& cbd)
{
  //- be sure the pointer to the dyn. attr. is valid
  if ( ! cbd.dya )
  {
    THROW_DEVFAILED("INTERNAL_ERROR",
                    "unexpected NULL pointer to dynamic attribute",
                    "DynamicInterface::write_callback");
  }

  //- log
  DEBUG_STREAM << "handling write request for dynamic attribute: "
               << cbd.dya->get_name()
               << std::endl;

  //- get access to the data associated with the dynamic attribute
  //- we choose to retreive it using the dyn. attr. "user data"
  Tango::DevShort * val = cbd.dya->get_user_data<Tango::DevShort>();

  //- be sure the pointer is valid
  if ( ! val )
  {
    TangoSys_OMemStream s;
    s << "unexpected NULL pointer to dynamic attribute data for "
      << cbd.dya->get_name();
    THROW_DEVFAILED("INTERNAL_ERROR", s.str().c_str(), "DynamicInterface::write_callback");
  }

  //- get the attribute value
  cbd.tga->get_write_value(*val);
}

//--------------------------------------------------------
/**
 *	Method      : DynamicInterface::get_device_property()
 *	Description : //	Add your own code to initialize
 */
//--------------------------------------------------------
void DynamicInterface::get_device_property()
{
	/*----- PROTECTED REGION ID(DynamicInterface::get_device_property_before) ENABLED START -----*/

	//	Initialize property data members

	/*----- PROTECTED REGION END -----*/	//	DynamicInterface::get_device_property_before

	/*----- PROTECTED REGION ID(DynamicInterface::get_device_property_after) ENABLED START -----*/

	//	Check device property data members init

	/*----- PROTECTED REGION END -----*/	//	DynamicInterface::get_device_property_after

}

//--------------------------------------------------------
/**
 *	Method      : DynamicInterface::always_executed_hook()
 *	Description : method always executed before any command is executed
 */
//--------------------------------------------------------
void DynamicInterface::always_executed_hook()
{
	/*----- PROTECTED REGION ID(DynamicInterface::always_executed_hook) ENABLED START -----*/

	//	code always executed before all requests

	/*----- PROTECTED REGION END -----*/	//	DynamicInterface::always_executed_hook
}

//--------------------------------------------------------
/**
 *	Method      : DynamicInterface::DynamicInterfaceClass::add_dynamic_attributes()
 *	Description : Create the dynamic attributes if any
 *	              for specified device.
 */
//--------------------------------------------------------
void DynamicInterface::add_dynamic_attributes()
{
	/*----- PROTECTED REGION ID(DynamicInterface::Class::add_dynamic_attributes) ENABLED START -----*/

	//	Add your own code to create and add dynamic attributes if any

	/*----- PROTECTED REGION END -----*/	//	DynamicInterface::Class::add_dynamic_attributes

}

//========================================================
//	Command execution methods
//========================================================
	/*----- PROTECTED REGION ID(DynamicInterface::namespace_ending) ENABLED START -----*/

	//	Additional Methods

	/*----- PROTECTED REGION END -----*/	//	DynamicInterface::namespace_ending
} //	namespace
