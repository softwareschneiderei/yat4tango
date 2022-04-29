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
//		TANGO Project - DeviceTask example
//
// = File
//		GroupManagerTask.cpp
//
// = AUTHOR
//		N.Leclercq - SOLEIL & JC.Pret - APSIDE
//
// ============================================================================

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <math.h>
#include <yat/threading/Mutex.h>
#include "GroupManagerTask.h"

// ============================================================================
// SOME USER DEFINED MESSAGES
// ============================================================================
#define kSTART_PERIODIC_MSG (yat::FIRST_USER_MSG + 1000)
#define kSTOP_PERIODIC_MSG  (yat::FIRST_USER_MSG + 1001)
#define kDATA_MSG           (yat::FIRST_USER_MSG + 1002)

namespace GroupManager_ns
{

// ============================================================================
// GroupManagerTask::GroupManagerTask
// ============================================================================
GroupManagerTask::GroupManagerTask (const Config & cfg)
  : yat4tango::DeviceTask(cfg.host_device), 
    m_cfg(cfg)
{
  //- configure optional msg handling
  this->enable_timeout_msg(false);
  this->enable_periodic_msg(false);
  this->set_periodic_msg_period(cfg.task_activity_period_ms);
}

// ============================================================================
// GroupManagerTask::~GroupManagerTask
// ============================================================================
GroupManagerTask::~GroupManagerTask ()
{
  //- noop
}

// ============================================================================
// GroupManagerTask::process_message
// ============================================================================
void GroupManagerTask::process_message (yat::Message& msg)
{
  //- handle msg
  switch (msg.type())
  {
    //- THREAD_INIT ----------------------
    case yat::TASK_INIT:
      {
	      DEBUG_STREAM << "GroupManagerTask::handle_message::THREAD_INIT::thread is starting up" << std::endl;
      } 
	    break;
      
	  //- THREAD_EXIT ----------------------
	  case yat::TASK_EXIT:
	    {
			  DEBUG_STREAM << "GroupManagerTask::handle_message::THREAD_EXIT::thread is quitting" << std::endl;
      }
		  break;
      
	  //- THREAD_PERIODIC ------------------
	  case yat::TASK_PERIODIC:
	    {
        DEBUG_STREAM << "GroupManagerTask::handle_message::THREAD_PERIODIC" << std::endl;
		    this->periodic_job_i();
      }
	    break;
      
	  //- THREAD_TIMEOUT -------------------
	  case yat::TASK_TIMEOUT:
	    {
        //- not used in this example
      }
      break;

    //- kSTART_PERIODIC_MSG --------------
    case kSTART_PERIODIC_MSG:
      {
    	  DEBUG_STREAM << "GroupManagerTask::handle_message::kSTART_PERIODIC_MSG" << std::endl;
        this->enable_periodic_msg(true);
      }
      break;

    //- kSTOP_PERIODIC_MSG ---------------
    case kSTOP_PERIODIC_MSG:
      {
    	  DEBUG_STREAM << "GroupManagerTask::handle_message::kSTOP_PERIODIC_MSG" << std::endl;
        this->enable_periodic_msg(false);
      }
      break;

    //- kDATA_MSG ------------------------
    case kDATA_MSG:
      {
        //- expecting a double (might throw an exception in case the msg encapsulates the wrong data type)
        try
        {
          double & d = msg.get_data<double>();
    	    DEBUG_STREAM << "GroupManagerTask::handle_message::kDATA_MSG::received value: " <<  d << std::endl;
        }
        catch (yat::Exception& ye)
        {
          _YAT_TO_TANGO_EXCEPTION(ye, te);
          ERROR_STREAM << te << std::endl;
        }
      }
      break;

    //- UNHANDLED MSG --------------------
		default:
		  DEBUG_STREAM << "GroupManagerTask::handle_message::unhandled msg type received" << std::endl;
			break;
  }
}

// ============================================================================
// GroupManagerTask::start_periodic_activity
// ============================================================================
void GroupManagerTask::start_periodic_activity (size_t tmo_ms)
{
  try
  {
    //- synchronous approach: "post then wait for the message to be handled"
    if (tmo_ms)
      this->wait_msg_handled(kSTART_PERIODIC_MSG, tmo_ms);
    //- asynchronous approach: "post the data then return immediately"
    else
      this->post(kSTART_PERIODIC_MSG);
  }
  catch (const Tango::DevFailed&)
  {
    //- an exception could be thrown if the task msgQ is full (high water mark reached)
    //- in the synchronous case we could also caught an exception thrown by the code 
    //- handling the message 
    throw;
  }
}

// ============================================================================
// GroupManagerTask::stop_periodic_activity
// ============================================================================
void GroupManagerTask::stop_periodic_activity (size_t tmo_ms)
{
  try
  {
    //- synchronous approach: "post then wait for the message to be handled"
    if (tmo_ms)
      this->wait_msg_handled(kSTOP_PERIODIC_MSG, tmo_ms);
    //- asynchronous approach: "post the data then returm immediately"
    else
      this->post(kSTOP_PERIODIC_MSG);
  }
  catch (const Tango::DevFailed&)
  {
    //- an exception could be thrown if the task msgQ is full (high water mark reached)
    //- in the synchronous case we could also caught an exception thrown by the code 
    //- handling the message 
    throw;
  }
}

// ============================================================================
// GroupManagerTask::post_some_data_to_the_task
// ============================================================================
void GroupManagerTask::post_some_data_to_the_task (double the_data, size_t tmo_ms)
{
  try
  {
    //- synchronous approach: "post the data then wait for the message to be handled"
    if (tmo_ms)
      this->wait_msg_handled(kDATA_MSG, the_data, tmo_ms);
    //- asynchronous approach: "post the data then returm immediately"
    else
      this->post(kDATA_MSG, the_data, 100);
  }
  catch (const Tango::DevFailed&)
  {
    //- an exception could be thrown if the task msgQ is full (high water mark reached)
    //- in the synchronous case we could also caught an exception thrown by the code 
    //- handling the message 
    throw;
  }
} 

// ============================================================================
// GroupManagerTask::periodic_job_i
// ============================================================================
void GroupManagerTask::periodic_job_i ()
{
  //- TODO
} 

} // namespace
