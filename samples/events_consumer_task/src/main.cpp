//----------------------------------------------------------------------------
// Copyright (c) 2004-2015 Synchrotron SOLEIL
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Lesser Public License v3
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/lgpl.html
//----------------------------------------------------------------------------
#include <iostream>
#include <yat4tango/LogHelper.h>
#include <yat4tango/ExceptionHelper.h>
#include <yat4tango/EventsConsumerTask.h>

//-----------------------------------------------------------------------------
// EVENTS MSGs
//-----------------------------------------------------------------------------
static const size_t EVENT_1 = yat::FIRST_USER_MSG + 1;
static const size_t EVENT_2 = yat::FIRST_USER_MSG + 2;
static const size_t EVENT_3 = yat::FIRST_USER_MSG + 3;
static const size_t EVENT_4 = yat::FIRST_USER_MSG + 4;
static const size_t EVENT_5 = yat::FIRST_USER_MSG + 5;
static const size_t EVENT_6 = yat::FIRST_USER_MSG + 6;

//-----------------------------------------------------------------------------
// MAIN
//-----------------------------------------------------------------------------
class MyEventsConsumerTask : public yat4tango::EventsConsumerTask
{
public:

  //--------------------------------------------------------
  MyEventsConsumerTask ()
    : yat4tango::EventsConsumerTask(0)
  {
    //- noop
  }

  //--------------------------------------------------------
  virtual ~MyEventsConsumerTask ()
  {
    std::cout << "MyEventsConsumerTask::dtor" << std::endl;
  }

  //--------------------------------------------------------
  void init ()
  {
    yat4tango::EventSubscription es;
    es.form.dev_name  = "sys/tg_test/1";
    es.form.user_data = 0;
    es.form.stateless = true;
    //-------------------------------------------
    es.form.obj_name = "double_scalar";
    es.form.evt_type = Tango::CHANGE_EVENT;
    es.form.msg_type = EVENT_1;
    this->subscribe_event(es);
    //-------------------------------------------
    es.form.obj_name = "float_scalar";
    es.form.evt_type = Tango::PERIODIC_EVENT;
    es.form.msg_type = EVENT_2;
    this->subscribe_event(es);
    //-------------------------------------------
    es.form.obj_name = "long_scalar";
    es.form.evt_type = Tango::CHANGE_EVENT;
    es.form.msg_type = EVENT_3;
    this->subscribe_event(es);
    //-------------------------------------------
    es.form.obj_name  = "short_scalar";
    es.form.evt_type  = Tango::PERIODIC_EVENT;
    es.form.msg_type  = EVENT_4;
    es.action.type = yat4tango::SubscriptionAction::AUTO_CONFIGURE;
    es.action.polling_period_ms = 250;
    es.action.period_ms = 250;
    this->subscribe_event(es);
    //-------------------------------------------
    es.form.obj_name  = "ushort_scalar";
    es.form.evt_type  = Tango::CHANGE_EVENT;
    es.form.msg_type  = EVENT_5;
    es.action.type = yat4tango::SubscriptionAction::AUTO_CONFIGURE;
    es.action.polling_period_ms = 150;
    es.action.absolute_change = 0;
    es.action.relative_change = 1;
    this->subscribe_event(es);
    //-------------------------------------------
    es.form.obj_name  = "double_scalar";
    es.form.evt_type  = Tango::ATTR_CONF_EVENT;
    es.form.msg_type  = EVENT_6;
    this->subscribe_event(es);
  }

  //--------------------------------------------------------
  virtual void process_message (yat::Message& msg)
  {
    switch ( msg.type() )
    {
      //- TASK_INIT ---------------------------------------------------------------------------
      case yat::TASK_INIT:
        {
          std::cout << "MyEventsConsumerTask::process_message::TASK_INIT" << std::endl;
          this->init();
        }
        break;
      //- TASK_EXIT ---------------------------------------------------------------------------
      case yat::TASK_EXIT:
        {
          std::cout << "MyEventsConsumerTask::process_message::TASK_EXIT" << std::endl;
        }
        break;
      //---------------------------------------------------------------------------------------
      //- EVENT_1 -----------------------------------------------------------------------------
      //---------------------------------------------------------------------------------------
      case EVENT_1:
        {
          std::cout << "MyEventsConsumerTask::process_message::EVENT_1" << std::endl;
          yat4tango::EventData & ed = msg.get_data<yat4tango::EventData>();
          std::cout << "event-data.dev_name..........." << ed.dev_name         << std::endl;
          std::cout << "event-data.attr_name.........." << ed.attr_name        << std::endl;
          std::cout << "event-data.attr_data_format..." << ed.attr_data_format << std::endl;
          std::cout << "event-data.attr_data_type....." << ed.attr_data_type   << std::endl;
          std::cout << "event-data.evt_type..........." << ed.evt_type         << std::endl;
          std::cout << "event-data.evt_id............." << ed.evt_id           << std::endl;
          std::cout << "event-data.has_error.........." << ed.has_error        << std::endl;
          if ( ! ed.has_error && ed.attr_value && ! ed.attr_value->is_empty() )
          {
            //- EVENT-1 is associated to double_scalar
            Tango::DevDouble v;
            *(ed.attr_value) >> v;
            std::cout << "event-data.attr.value........." << v << std::endl;
          }
        }
        break;
      //---------------------------------------------------------------------------------------
      //- EVENT_2 -----------------------------------------------------------------------------
      //---------------------------------------------------------------------------------------
      case EVENT_2:
        {
          std::cout << "MyEventsConsumerTask::process_message::EVENT_2" << std::endl;
          yat4tango::EventData & ed = msg.get_data<yat4tango::EventData>();
          std::cout << "event-data.dev_name..........." << ed.dev_name         << std::endl;
          std::cout << "event-data.attr_name.........." << ed.attr_name        << std::endl;
          std::cout << "event-data.attr_data_format..." << ed.attr_data_format << std::endl;
          std::cout << "event-data.attr_data_type....." << ed.attr_data_type   << std::endl;
          std::cout << "event-data.evt_type..........." << ed.evt_type         << std::endl;
          std::cout << "event-data.evt_id............." << ed.evt_id           << std::endl;
          std::cout << "event-data.has_error.........." << ed.has_error        << std::endl;
          if ( ! ed.has_error && ed.attr_value && ! ed.attr_value->is_empty() )
          {
            //- EVENT-2 is associated to float_scalar
            Tango::DevFloat v;
            *(ed.attr_value) >> v;
            std::cout << "event-data.attr.value........." << v << std::endl;
          }
        }
        break;
      //---------------------------------------------------------------------------------------
      //- EVENT_3 -----------------------------------------------------------------------------
      //---------------------------------------------------------------------------------------
      case EVENT_3:
        {
          std::cout << "MyEventsConsumerTask::process_message::EVENT_3" << std::endl;
          yat4tango::EventData & ed = msg.get_data<yat4tango::EventData>();
          std::cout << "event-data.dev_name..........." << ed.dev_name         << std::endl;
          std::cout << "event-data.attr_name.........." << ed.attr_name        << std::endl;
          std::cout << "event-data.attr_data_format..." << ed.attr_data_format << std::endl;
          std::cout << "event-data.attr_data_type....." << ed.attr_data_type   << std::endl;
          std::cout << "event-data.evt_type..........." << ed.evt_type         << std::endl;
          std::cout << "event-data.evt_id............." << ed.evt_id           << std::endl;
          std::cout << "event-data.has_error.........." << ed.has_error        << std::endl;
          if ( ! ed.has_error && ed.attr_value && ! ed.attr_value->is_empty() )
          {
            //- EVENT-3 is associated to long_scalar
            Tango::DevLong v;
            *(ed.attr_value) >> v;
            std::cout << "event-data.attr.value........." << v << std::endl;
          }
        }
        break;
      //---------------------------------------------------------------------------------------
      //- EVENT_4 -----------------------------------------------------------------------------
      //---------------------------------------------------------------------------------------
      case EVENT_4:
        {
          std::cout << "MyEventsConsumerTask::process_message::EVENT_4" << std::endl;
          yat4tango::EventData & ed = msg.get_data<yat4tango::EventData>();
          std::cout << "event-data.dev_name..........." << ed.dev_name         << std::endl;
          std::cout << "event-data.attr_name.........." << ed.attr_name        << std::endl;
          std::cout << "event-data.attr_data_format..." << ed.attr_data_format << std::endl;
          std::cout << "event-data.attr_data_type....." << ed.attr_data_type   << std::endl;
          std::cout << "event-data.evt_type..........." << ed.evt_type         << std::endl;
          std::cout << "event-data.evt_id............." << ed.evt_id           << std::endl;
          std::cout << "event-data.has_error.........." << ed.has_error        << std::endl;
          if ( ! ed.has_error && ed.attr_value && ! ed.attr_value->is_empty() )
          {
            //- EVENT-4 is associated to short_scalar
            Tango::DevShort v;
            *(ed.attr_value) >> v;
            std::cout << "event-data.attr.value........." << v << std::endl;
          }
        }
        break;
      //---------------------------------------------------------------------------------------
      //- EVENT_5 -----------------------------------------------------------------------------
      //---------------------------------------------------------------------------------------
      case EVENT_5:
        {
          std::cout << "MyEventsConsumerTask::process_message::EVENT_5" << std::endl;
          yat4tango::EventData & ed = msg.get_data<yat4tango::EventData>();
          std::cout << "event-data.dev_name..........." << ed.dev_name         << std::endl;
          std::cout << "event-data.attr_name.........." << ed.attr_name        << std::endl;
          std::cout << "event-data.attr_data_format..." << ed.attr_data_format << std::endl;
          std::cout << "event-data.attr_data_type....." << ed.attr_data_type   << std::endl;
          std::cout << "event-data.evt_type..........." << ed.evt_type         << std::endl;
          std::cout << "event-data.evt_id............." << ed.evt_id           << std::endl;
          std::cout << "event-data.has_error.........." << ed.has_error        << std::endl;
          if ( ! ed.has_error && ed.attr_value && ! ed.attr_value->is_empty() )
          {
            //- EVENT-5 is associated to ushort_scalar
            Tango::DevUShort v;
            *(ed.attr_value) >> v;
            std::cout << "event-data.attr.value........." << v << std::endl;
          }
        }
        break;
      //---------------------------------------------------------------------------------------
      //- EVENT_6 -----------------------------------------------------------------------------
      //---------------------------------------------------------------------------------------
      case EVENT_6:
        {
          std::cout << "MyEventsConsumerTask::process_message::EVENT_5" << std::endl;
          yat4tango::AttributeConfigEventData & ed = msg.get_data<yat4tango::AttributeConfigEventData>();
          std::cout << "event-data.dev_name..........." << ed.dev_name         << std::endl;
          std::cout << "event-data.attr_name.........." << ed.attr_name        << std::endl;
          std::cout << "event-data.evt_id............." << ed.evt_id           << std::endl;
          std::cout << "event-data.has_error.........." << ed.has_error        << std::endl;
          if ( ! ed.has_error && ed.attr_conf )
          {
            std::cout << "event-data.label.............." << ed.attr_conf->label         << std::endl;
            std::cout << "event-data.unit..............." << ed.attr_conf->unit          << std::endl;
            std::cout << "event-data.standard_unit......" << ed.attr_conf->standard_unit << std::endl;
            std::cout << "event-data.display_unit......." << ed.attr_conf->display_unit  << std::endl;
            std::cout << "event-data.format............." << ed.attr_conf->format        << std::endl;
            std::cout << "event-data.min_value.........." << ed.attr_conf->min_value     << std::endl;
            std::cout << "event-data.max_value.........." << ed.attr_conf->max_value     << std::endl;
            std::cout << "event-data.min_alarm.........." << ed.attr_conf->min_alarm     << std::endl;
            std::cout << "event-data.max_alarm.........." << ed.attr_conf->max_alarm     << std::endl;
          }
        }
        break;
      //------------------------------------
      default:
        break;
    }
  }
};

//-----------------------------------------------------------------------------
// MAIN
//-----------------------------------------------------------------------------
int main (int argc, char* argv[])
{
  MyEventsConsumerTask * t = new MyEventsConsumerTask();

  try
  {
    std::cout << "EventsConsumerTask test..." << std::endl;
   
    t->go_synchronously(30000);

    for ( size_t i = 0; i < 10 ; i++ )
      yat::Thread::sleep(1000);

    std::cout << "EventsConsumerTask::test passed!" << std::endl;
  }
  catch ( Tango::DevFailed & e )
  {
    Tango::Except::print_exception(e);
  }
  catch ( ... )
  {
    std::cout << "unknown exception caught!" << std::endl;
  }
  
  t->exit();
  
  return 0;
}
