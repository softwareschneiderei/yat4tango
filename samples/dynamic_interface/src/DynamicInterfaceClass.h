//----------------------------------------------------------------------------
// Copyright (c) 2004-2015 Synchrotron SOLEIL
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Lesser Public License v3
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/lgpl.html
//----------------------------------------------------------------------------
//
// file :        DynamicInterfaceClass.h
//
// description : Include for the DynamicInterfaceClass root class.
//               This class is the singleton class for.
//               the DynamicInterface device class..
//               It contains all properties and methods which the .
//               DynamicInterface requires only once e.g. the commands.
//
// project :     DynamicInterface.
//
// $Author:  $
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


#ifndef DynamicInterfaceCLASS_H
#define DynamicInterfaceCLASS_H

#include <tango.h>
#include <DynamicInterface.h>

/*----- PROTECTED REGION END -----*/

namespace DynamicInterface_ns
{
	/*----- PROTECTED REGION ID(DynamicInterface::classes for dynamic creation) ENABLED START -----*/

	/*----- PROTECTED REGION END -----*/	//	DynamicInterface::classes for dynamic creation



//=========================================
//	Define classes for attributes
//=========================================




//=========================================
//	Define classes for commands
//=========================================




/**
 *	The TemplateDevServClass singleton definition
 */

class
#ifdef _TG_WINDOWS_
	__declspec(dllexport)
#endif
	DynamicInterfaceClass : public Tango::DeviceClass
{
	/*----- PROTECTED REGION ID(DynamicInterface::Additionnal DServer data members) ENABLED START -----*/

	/*----- PROTECTED REGION END -----*/	//	DynamicInterface::Additionnal DServer data members



public:
//	write class properties data members
	Tango::DbData	cl_prop;
	Tango::DbData	cl_def_prop;
	Tango::DbData	dev_def_prop;

//	Method prototypes
	static DynamicInterfaceClass *init(const char *);
	static DynamicInterfaceClass *instance();
	~DynamicInterfaceClass();
	Tango::DbDatum	get_class_property(string &);
	Tango::DbDatum	get_default_device_property(string &);
	Tango::DbDatum	get_default_class_property(string &);
	
protected:
	DynamicInterfaceClass(string &);
	static DynamicInterfaceClass *_instance;
	void command_factory();
	void attribute_factory(vector<Tango::Attr *> &);
	void write_class_property();
	void set_default_property();
	void get_class_property();
	string get_cvstag();
	string get_cvsroot();

private:
	void device_factory(const Tango::DevVarStringArray *);
	void create_static_attribute_list(vector<Tango::Attr *> &);
	void erase_dynamic_attributes(const Tango::DevVarStringArray *,vector<Tango::Attr *> &);
	vector<string>	defaultAttList;


};

}	//	namespace

#endif	//	DynamicInterfaceCLASS_H

