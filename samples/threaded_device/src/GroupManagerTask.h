//----------------------------------------------------------------------------
// Copyright (c) 2004-2015 Synchrotron SOLEIL
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Lesser Public License v3
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/lgpl.html
//----------------------------------------------------------------------------
// ============================================================================
//
// = CONTEXT
//		DeviceTask example
//
// = File
//		GroupManagerTask.h
//
// = AUTHOR
//    N.Leclercq - SOLEIL & JC.Pret - APSIDE
//
// ============================================================================

#ifndef _MY_DEVICE_TASK_H_
#define _MY_DEVICE_TASK_H_

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <tango.h>

#include <yat/any/GenericContainer.h>
#include <yat/memory/DataBuffer.h>
#include <yat4tango/DeviceTask.h>
#include <yat4tango/DynamicAttributeManager.h>

namespace GroupManager_ns
{

// ============================================================================
// class: GroupManagerTask
// ============================================================================
class GroupManagerTask : public yat4tango::DeviceTask
{
public:

  //- the task's configuration data struct
  typedef struct Config 
  {
    //-default ctor
    Config ()
      : task_activity_period_ms(1000), 
        host_device(0)
    {}
    //- copy ctor
    Config (const Config & src)
    { 
      *this = src;
    }
    //- operator=
    const Config & operator= (const Config & src)
    { 
      if (&src == this)
        return *this;
      task_activity_period_ms = src.task_activity_period_ms;
      host_device = src.host_device;
      return *this;
    }
    //- period of task's periodic activity in msecs 
    size_t task_activity_period_ms;
    //- the Tango device hoting this task
    Tango::DeviceImpl * host_device;
  } Config;

	//- ctor
	GroupManagerTask (const Config & cfg);

	//- dtor
	virtual ~GroupManagerTask ();

  //- starts task's periodic activity (a <tmo_ms> of 0 means asynchronous exec)
  void start_periodic_activity (size_t tmo_ms = 0);

  //- stops task's periodic activity (a <tmo_ms> of 0 means asynchronous exec)
  void stop_periodic_activity (size_t tmo_ms = 0);

  //- post some data to the task (a <tmo_ms> of 0 means asynchronous exec)
  void post_some_data_to_the_task (double the_data, size_t tmo_ms = 0);

protected:
	//- process_message (implements yat4tango::DeviceTask pure virtual method)
	virtual void process_message (yat::Message& msg);

private:
  //- encapsulates the periodic activity
  void periodic_job_i ();

  //- the task's configuration 
  Config m_cfg;
};

} // namespace GroupManager_ns

#endif // _MY_DEVICE_TASK_H_
