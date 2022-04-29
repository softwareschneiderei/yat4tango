//----------------------------------------------------------------------------
// Copyright (c) 2004-2015 Synchrotron SOLEIL
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Lesser Public License v3
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/lgpl.html
//----------------------------------------------------------------------------
//=============================================================================
//
// file :         GroupManagerClass.h
//
// description :  Include for the GroupManagerClass root class.
//                This class is represents the singleton class for
//                the GroupManager device class.
//                It contains all properties and methods which the 
//                GroupManager requires only once e.g. the commands.
//			
// project :      TANGO Device Server
//
// $Author: leclercq $
//
// $Revision: 1.1 $
//
// $Log: GroupManagerClass.h,v $
// Revision 1.1  2007/07/18 13:09:43  leclercq
// no message
//
// Revision 1.3  2007/07/18 13:02:50  leclercq
// Added a threaded device example
//
// Revision 1.1  2007/07/18 12:57:05  leclercq
// Added a threaded device example
//
//
// copyleft :     European Synchrotron Radiation Facility
//                BP 220, Grenoble 38043
//                FRANCE
//
//=============================================================================
//
//  		This file is generated by POGO
//	(Program Obviously used to Generate tango Object)
//
//         (c) - Software Engineering Group - ESRF
//=============================================================================

#ifndef _GroupManagerCLASS_H
#define _GroupManagerCLASS_H

#include <tango.h>
#include <GroupManager.h>


namespace GroupManager_ns
{
//=====================================
//	Define classes for attributes
//=====================================
class postThisDoubleToTheTaskAttrib: public Tango::Attr
{
public:
	postThisDoubleToTheTaskAttrib():Attr("postThisDoubleToTheTask", Tango::DEV_DOUBLE, Tango::READ_WRITE) {};
	~postThisDoubleToTheTaskAttrib() {};
	
	virtual void read(Tango::DeviceImpl *dev,Tango::Attribute &att)
	{(static_cast<GroupManager *>(dev))->read_postThisDoubleToTheTask(att);}
	virtual void write(Tango::DeviceImpl *dev,Tango::WAttribute &att)
	{(static_cast<GroupManager *>(dev))->write_postThisDoubleToTheTask(att);}
	virtual bool is_allowed(Tango::DeviceImpl *dev,Tango::AttReqType ty)
	{return (static_cast<GroupManager *>(dev))->is_postThisDoubleToTheTask_allowed(ty);}
};

//=========================================
//	Define classes for commands
//=========================================
class StopClass : public Tango::Command
{
public:
	StopClass(const char   *name,
	               Tango::CmdArgType in,
				   Tango::CmdArgType out,
				   const char        *in_desc,
				   const char        *out_desc,
				   Tango::DispLevel  level)
	:Command(name,in,out,in_desc,out_desc, level)	{};

	StopClass(const char   *name,
	               Tango::CmdArgType in,
				   Tango::CmdArgType out)
	:Command(name,in,out)	{};
	~StopClass() {};
	
	virtual CORBA::Any *execute (Tango::DeviceImpl *dev, const CORBA::Any &any);
	virtual bool is_allowed (Tango::DeviceImpl *dev, const CORBA::Any &any)
	{return (static_cast<GroupManager *>(dev))->is_Stop_allowed(any);}
};



class StartClass : public Tango::Command
{
public:
	StartClass(const char   *name,
	               Tango::CmdArgType in,
				   Tango::CmdArgType out,
				   const char        *in_desc,
				   const char        *out_desc,
				   Tango::DispLevel  level)
	:Command(name,in,out,in_desc,out_desc, level)	{};

	StartClass(const char   *name,
	               Tango::CmdArgType in,
				   Tango::CmdArgType out)
	:Command(name,in,out)	{};
	~StartClass() {};
	
	virtual CORBA::Any *execute (Tango::DeviceImpl *dev, const CORBA::Any &any);
	virtual bool is_allowed (Tango::DeviceImpl *dev, const CORBA::Any &any)
	{return (static_cast<GroupManager *>(dev))->is_Start_allowed(any);}
};



//
// The GroupManagerClass singleton definition
//

class
#ifdef _TG_WINDOWS_
	__declspec(dllexport)
#endif
	GroupManagerClass : public Tango::DeviceClass
{
public:
//	properties member data

//	add your own data members here
//------------------------------------

public:
	Tango::DbData	cl_prop;
	Tango::DbData	cl_def_prop;
	Tango::DbData	dev_def_prop;

//	Method prototypes
	static GroupManagerClass *init(const char *);
	static GroupManagerClass *instance();
	~GroupManagerClass();
	Tango::DbDatum	get_class_property(string &);
	Tango::DbDatum	get_default_device_property(string &);
	Tango::DbDatum	get_default_class_property(string &);
	
protected:
	GroupManagerClass(string &);
	static GroupManagerClass *_instance;
	void command_factory();
	void get_class_property();
	void attribute_factory(vector<Tango::Attr *> &);
	void write_class_property();
	void set_default_property();
	string get_cvstag();
	string get_cvsroot();

private:
	void device_factory(const Tango::DevVarStringArray *);
};


}	//	namespace GroupManager_ns

#endif // _GROUPMANAGERCLASS_H
