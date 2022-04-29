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
#include <yat/utils/XString.h>
#include <yat4tango/ExceptionHelper.h>
#include <yat4tango/EventsConsumerTask.h>

#if (TANGO_VERSION_MAJOR >= 8)

namespace yat4tango
{

// ======================================================================
// _LOCAL_TRACES_
// ======================================================================
//- #define _LOCAL_TRACES_

// ======================================================================
// MINIMUM ALLOWED POLLING PERIOD IN MS (AUTO-CONF EVENT)
// ======================================================================
#define PP_THRESHOLD_IN_MS 100

// ======================================================================
// EventsConsumerTask::EventsConsumerTask
// ======================================================================
EventsConsumerTask::EventsConsumerTask (Tango::DeviceImpl * _dev)
  : DeviceTask (_dev),
    Tango::CallBack(),
    tango_evts_post_tmo_ms(500)
{
  YAT_TRACE("EventsConsumerTask::EventsConsumerTask");
}

// ======================================================================
// EventsConsumerTask::~EventsConsumerTask
// ======================================================================
EventsConsumerTask::~EventsConsumerTask ()
{
  YAT_TRACE("EventsConsumerTask::~EventsConsumerTask");

  this->unsubscribe_events();
}

// ======================================================================
// EventsConsumerTask::exit
// ======================================================================
void EventsConsumerTask::exit ()
{
  this->unsubscribe_events();

  DeviceTask::exit();
}

// ======================================================================
// EventsConsumerTask::push_event
// ======================================================================
void EventsConsumerTask::push_event (Tango::EventData * evt_data)
{
  if ( ! evt_data )
    return;

  Tango::EventType evt_type = Tango::PERIODIC_EVENT;
  
  if ( evt_data->event == "change" )
  {
    evt_type = Tango::CHANGE_EVENT;
  }
  else if ( evt_data->event == "user" )
  {
    evt_type = Tango::USER_EVENT;
  }

  try
  {
    const EventInfo & ei = this->event_info_i(evt_data->attr_name, evt_type);

    EventData * ed = new EventData;
    if ( ! ed )
    {
      //- do not event try to post a message in this case!
      return;
    }
    
    this->initialize_event_data(*ed, ei);
    ed->dev_proxy = evt_data->device;
    ed->has_error = evt_data->err;

    if ( ed->has_error )
    {
      ed->errors = evt_data->errors;
      ed->attr_value = 0;
    }
    else
    {
      ed->attr_value = evt_data->attr_value;
      evt_data->attr_value = 0;
    }

    this->post(ei.msg_type, ed, this->tango_evts_post_tmo_ms);
  }
  catch ( ... )
  {
    //...
  }
}

// ======================================================================
// EventsConsumerTask::push_event
// ======================================================================
void EventsConsumerTask::push_event (Tango::AttrConfEventData * evt_data)
{
  if ( ! evt_data )
    return;

  size_t msg_id = 0;

  try
  {
    const EventInfo & ei = this->event_info_i(evt_data->attr_name, Tango::ATTR_CONF_EVENT);

    AttributeConfigEventData * ed = new AttributeConfigEventData;
    if ( ! ed )
    {
      //- do not event try to post a message in this case!
      return;
    }
  
    this->initialize_event_data(*ed, ei);
    ed->dev_proxy = evt_data->device;
    ed->has_error = evt_data->err;
    if ( ed->has_error )
    {
      ed->errors = evt_data->errors;
      ed->attr_conf = 0;
    }
    else
    {
      ed->attr_conf = evt_data->attr_conf;
      evt_data->attr_conf = 0;
    }

    this->post(ei.msg_type, ed, this->tango_evts_post_tmo_ms);
  }
  catch ( ... )
  {
    //...
  }
}

// ======================================================================
// EventsConsumerTask::push_event
// ======================================================================
void EventsConsumerTask::push_event (Tango::DataReadyEventData * evt_data)
{
  if ( ! evt_data )
    return;
  
  try
  {
    const EventInfo & ei = this->event_info_i(evt_data->attr_name, Tango::DATA_READY_EVENT);
    
    DataReadyEventData * ed = new DataReadyEventData;
    if ( ! ed )
    {
      //- do not event try to post a message in this case!
      return;
    }

    this->initialize_event_data(*ed, ei);
    ed->dev_proxy = evt_data->device;
    ed->has_error = evt_data->err;
    if ( ed->has_error )
    {
      ed->errors = evt_data->errors;
      ed->ctr = 0;
    }
    else
    {
      ed->ctr = evt_data->ctr;
    }
    
    this->post(ei.msg_type, ed, this->tango_evts_post_tmo_ms);
  }
  catch ( ... )
  {
    //...
  }
}

// ======================================================================
// EventsConsumerTask::event_subscribed
// ======================================================================
bool EventsConsumerTask::event_subscribed (const std::string& dev_name,
                                           const std::string& attr_name,
                                           Tango::EventType evt_type,
                                           int * evt_id)
{
  yat::MutexLock guard(this->subscribed_events_lock);
    
  return this->event_subscribed_i(dev_name, attr_name, evt_type, evt_id);
}

// ======================================================================
// EventsConsumerTask::event_subscribed
// ======================================================================
bool EventsConsumerTask::event_subscribed (int evt_id)
{
  yat::MutexLock guard(this->subscribed_events_lock);

  return this->subscribed_events.find(evt_id) != this->subscribed_events.end();
}

// ======================================================================
// EventsConsumerTask::event_subscribed_i
// ======================================================================
bool EventsConsumerTask::event_subscribed_i (const std::string& dev_name,
                                             const std::string& attr_name,
                                             Tango::EventType evt_type,
                                             int * evt_id)
{
  EventInfoMapIterator it  = this->subscribed_events.begin();
  EventInfoMapIterator end = this->subscribed_events.end();
  
  for ( ; it != end; ++it )
  {
    bool src_matches = it->second.evt_src == dev_name + "/" + attr_name;
    bool evt_matches = it->second.evt_type == evt_type;
    if ( src_matches && evt_matches )
    {
      if ( evt_id )
      {
        *evt_id = it->second.evt_id;
      }
      return true;
    }
  }
  
  if ( evt_id )
  {
    *evt_id = -1;
  }
  return false;
}

// ======================================================================
// EventsConsumerTask::message_identifier_i
// ======================================================================
int EventsConsumerTask::message_identifier_i (const std::string& dev_name,
                                              const std::string& attr_name,
                                              Tango::EventType evt_type)
{
  EventInfoMapIterator it  = this->subscribed_events.begin();
  EventInfoMapIterator end = this->subscribed_events.end();
  
  for ( ; it != end; ++it )
  {
    bool src_matches = it->second.evt_src == dev_name + "/" + attr_name;
    bool evt_matches = evt_type == it->second.evt_type;
    if ( src_matches && evt_matches )
    {
      return it->second.msg_type;
    }
  }

  THROW_DEVFAILED("NO_SUCH_EVENT",
                  "the specified event could no be found",
                  "EventsConsumerTask::message_identifier_i");

  //- make the compiler happy
  return 0;
}

// ======================================================================
// EventsConsumerTask::message_identifier_i
// ======================================================================
int EventsConsumerTask::message_identifier_i (const std::string& evt_src,
                                              Tango::EventType evt_type)
{
  EventInfoMapIterator it  = this->subscribed_events.begin();
  EventInfoMapIterator end = this->subscribed_events.end();
  
  for ( ; it != end; ++it )
  {
    bool src_matches = evt_src.find(it->second.evt_src) != std::string::npos;
    bool evt_matches  = evt_type == it->second.evt_type;
    if ( src_matches && evt_matches )
    {
      return it->second.msg_type;
    }
  }
  
  THROW_DEVFAILED("NO_SUCH_EVENT",
                  "the specified event could no be found",
                  "EventsConsumerTask::message_identifier_i");

  //- make the compiler happy
  return 0;
}

// ======================================================================
// EventsConsumerTask::event_info_i
// ======================================================================
const EventsConsumerTask::EventInfo & EventsConsumerTask::event_info_i (const std::string& dev_name,
                                                                        const std::string& attr_name,
                                                                        Tango::EventType evt_type)
{
  EventInfoMapIterator it  = this->subscribed_events.begin();
  EventInfoMapIterator end = this->subscribed_events.end();

  for ( ; it != end; ++it )
  {
    bool src_matches = it->second.evt_src == dev_name + "/" + attr_name;
    bool evt_matches = evt_type == it->second.evt_type;
    if ( src_matches && evt_matches )
    {
      return it->second;
    }
  }

  THROW_DEVFAILED("NO_SUCH_EVENT",
                  "the specified event could no be found",
                  "EventsConsumerTask::event_info_i");

  //- make the compiler happy (safe cause will never be called)
  return this->subscribed_events[0];
}

// ======================================================================
// EventsConsumerTask::event_info_i
// ======================================================================
const EventsConsumerTask::EventInfo & EventsConsumerTask::event_info_i (const std::string& evt_src,
                                                                        Tango::EventType evt_type)
{
  EventInfoMapIterator it  = this->subscribed_events.begin();
  EventInfoMapIterator end = this->subscribed_events.end();

  for ( ; it != end; ++it )
  {
    bool src_matches = evt_src.find(it->second.evt_src) != std::string::npos;
    bool evt_matches  = evt_type == it->second.evt_type;
    if ( src_matches && evt_matches )
    {
      return it->second;
    }
  }

  THROW_DEVFAILED("NO_SUCH_EVENT",
                  "the specified event could no be found",
                  "EventsConsumerTask::event_info_i");

  //- make the compiler happy (safe cause will never be called)
  return this->subscribed_events[0];
}

// ======================================================================
// EventsConsumerTask::subscribe_event
// ======================================================================
int EventsConsumerTask::subscribe_event (const EventSubscription& es)
{
//TODO:CHECK SUBSCRIPTION EXIST FRO REQUESTED EVENT
//TODO:USE MAP OF MAPS (SEE PYTHON IMPL)
  
  EventInfo ei;
  
  try
  {
    //- main event info...
    ei.dev_name  = es.form.dev_name;
    ei.obj_name  = es.form.obj_name;
    ei.evt_src   = es.form.dev_name + "/" + es.form.obj_name;
    ei.evt_type  = es.form.evt_type;
    ei.msg_type  = es.form.msg_type;
    ei.user_data = es.form.user_data;

#if defined(_LOCAL_TRACES_)
    std::cout << "subscribing to " << ei.evt_src << std::endl;
#endif

    Tango::DeviceProxy * tdp = 0;

    //- already "subscribed" to an event from this device?
    DeviceProxyMapIterator it = this->proxies.find(ei.dev_name);
    if ( it != this->proxies.end() )
    {
      //- reuse proxy: increment its usage counter
      it->second.inc_usage_counter();
      tdp = it->second.proxy;
    }
    else
    {
      //- build a proxy to the target device
      tdp = new Tango::DeviceProxy(es.form.dev_name.c_str());
      if ( ! tdp )
      {
        THROW_DEVFAILED("MEMORY_ERROR",
                        "Tango::DeviceProxy instanciation failed [is the device registered into the Tango db?]",
                        "EventsConsumerTask::subscribe_event");
      }
      //- store the Tango device proxy locally (will be resued if required)
      DeviceProxy dp(tdp);
      this->proxies[ei.dev_name] = dp;
    }
    
    //- get attribute configuration
    Tango::AttributeInfoEx ai = tdp->get_attribute_config(es.form.obj_name);
      
    //- change or periodic event?
    bool subscribing_to_periodic_or_change_evt = es.form.evt_type == Tango::PERIODIC_EVENT
                                              || es.form.evt_type == Tango::CHANGE_EVENT;
                                  
    //- auto configure the event in case it's not configured into the db
    bool auto_conf = subscribing_to_periodic_or_change_evt
                        && es.action.type != SubscriptionAction::DO_NOTHING;

    //- is the polling enabled for the specified attribute?
    if ( auto_conf && ! tdp->is_attribute_polled(const_cast<std::string&>(es.form.obj_name)) )
    {
#if defined(_LOCAL_TRACES_)
      std::cout << "no polling on device side and auto-config on! configuring event..." << std::endl;
#endif
      //- check requested polling period (safety) 
      if ( es.action.polling_period_ms < PP_THRESHOLD_IN_MS )
      {
        std::ostringstream oss;
        oss << "Event auto-configuration error. For safety reason, the auto-configured polling period must >= "
            << PP_THRESHOLD_IN_MS
            << "ms [the specified value is "
            << es.action.polling_period_ms
            << "ms]";
#if defined(_LOCAL_TRACES_)
        std::cout <<  oss.str() << std::endl;
#endif
        THROW_DEVFAILED("EVENT_SUBSCRIPTION_ERROR",
                        oss.str().c_str(),
                        "EventsConsumerTask::subscribe_event");

      }
      
      //- check evt period versus polling period
      if ( es.action.period_ms < es.action.polling_period_ms )
      {
        std::ostringstream oss;
        oss << "Event auto-configuration error. The event period must be greater or equal to the polling period "
            << "[ep:"
            << es.action.period_ms
            << " - pp:"
            << es.action.polling_period_ms
            << "]";
#if defined(_LOCAL_TRACES_)
        std::cout <<  oss.str() << std::endl;
#endif
        THROW_DEVFAILED("EVENT_SUBSCRIPTION_ERROR",
                        oss.str().c_str(),
                        "EventsConsumerTask::subscribe_event");

      }
      
      //- periodic event
      if ( es.form.evt_type == Tango::PERIODIC_EVENT )
      {
        //- check evt period versus polling period
        if ( es.action.period_ms < es.action.polling_period_ms )
        {
          std::ostringstream oss;
          oss << "Event auto-configuration error. The event period must be greater or equal to the polling period "
              << "[ep:"
              << es.action.period_ms
              << " - pp:"
              << es.action.polling_period_ms
              << "]";
#if defined(_LOCAL_TRACES_)
          std::cout <<  oss.str() << std::endl;
#endif
          THROW_DEVFAILED("EVENT_SUBSCRIPTION_ERROR",
                          oss.str().c_str(),
                          "EventsConsumerTask::subscribe_event");
        }
        
        //- get/change/set attribute configuration
        ai.events.per_event.period = yat::XString<size_t>::to_string(es.action.polling_period_ms);
#if defined(_LOCAL_TRACES_)
        std::cout << "writting new attr. config into the Tango db - event period: "
                  << ai.events.per_event.period
                  << " ms"
                  << std::endl;
#endif
                  
        std::vector<Tango::AttributeInfoEx> ail;
        ail.push_back(ai);
        tdp->set_attribute_config(ail);
      }
      //- change event
      else if ( es.form.evt_type == Tango::CHANGE_EVENT )
      {
        //- check absolute change value
        if ( es.action.absolute_change == 0. && es.action.relative_change == 0. )
        {
          std::ostringstream oss;
          oss << "Event auto-configuration error. The both the absolute and relative change value are null.";
#if defined(_LOCAL_TRACES_)
          std::cout <<  oss.str() << std::endl;
#endif
          THROW_DEVFAILED("EVENT_SUBSCRIPTION_ERROR",
                          oss.str().c_str(),
                          "EventsConsumerTask::subscribe_event");
        }
        
        //- absolute change value
        ai.events.ch_event.abs_change = yat::XString<double>::to_string(es.action.absolute_change);
#if defined(_LOCAL_TRACES_)
        std::cout << "writting new attr. config into the Tango db - abs.change value: "
                  << ai.events.ch_event.abs_change
                  << " ms"
                  << std::endl;
#endif
        
        //- relative change value
        ai.events.ch_event.rel_change = yat::XString<double>::to_string(es.action.relative_change);
#if defined(_LOCAL_TRACES_)
        std::cout << "writting new attr. config into the Tango db - rel.change value: "
                  << ai.events.ch_event.rel_change
                  << " ms"
                  << std::endl;
#endif
        
        //- absolute change value      
        std::vector<Tango::AttributeInfoEx> ail;
        ail.push_back(ai);
        tdp->set_attribute_config(ail);    
      }
#if defined(_LOCAL_TRACES_)
      std::cout << "starting polling on server side - polling period: "
                << es.action.polling_period_ms
                << " ms"
                << std::endl;
#endif
                
      //- finally, enable polling on server side
      tdp->poll_attribute(const_cast<std::string&>(es.form.obj_name),
                          static_cast<int>(es.action.polling_period_ms));
#if defined(_LOCAL_TRACES_)
      std::cout << "evt-auto configuration done!" << std::endl;
#endif
    }
    
    //- save both data type and data format locally 
    ei.attr_data_type = ai.data_type;
    ei.attr_data_format = ai.data_format;
    
#if defined(_LOCAL_TRACES_)
    std::cout << "subscription request..." << std::endl;
#endif
                
    //- subscribe to the specified event
    ei.evt_id = tdp->subscribe_event(es.form.obj_name, es.form.evt_type, this, es.form.stateless);
    
#if defined(_LOCAL_TRACES_)
    std::cout << "subscription successfull!" << std::endl;
#endif
    
    //- store event info locally
    {
      yat::MutexLock guard(this->subscribed_events_lock);
      this->subscribed_events[ei.evt_id] = ei;
    }

#if defined(_LOCAL_TRACES_)
    std::cout << "subscription done!" << std::endl;
#endif
  }
  catch (Tango::DevFailed& e)
  {
#if defined(_LOCAL_TRACES_)
    std::cout << "Tango::DevFailed caught!" << std::endl;
#endif
    if ( this->proxies.find(ei.dev_name) != this->proxies.end() && ! this->proxies[ei.dev_name].dec_usage_counter() )
    {
      this->proxies[ei.dev_name].release();
      this->proxies.erase(ei.dev_name);
    }
    RETHROW_DEVFAILED(e,
                      "EVENT_SUBSCRIPTION_ERROR",
                      "failed to subscribe to the specified Tango event [Tango error caught]",
                      "EventsConsumerTask::subscribe_event");
  }
  catch (...)
  {
#if defined(_LOCAL_TRACES_)
    std::cout << "unknown exception caught!" << std::endl;
#endif
    if ( this->proxies.find(ei.dev_name) != this->proxies.end() && ! this->proxies[ei.dev_name].dec_usage_counter() )
    {
      this->proxies[ei.dev_name].release();
      this->proxies.erase(ei.dev_name);
    }
    THROW_DEVFAILED("EVENT_SUBSCRIPTION_ERROR",
                    "failed to subscribe to the specified Tango event [unknown error caught]",
                    "EventsConsumerTask::subscribe_event");
  }

  return ei.evt_id;
}

// ======================================================================
// EventsConsumerTask::unsubscribe_event
// ======================================================================
void EventsConsumerTask::unsubscribe_event (const std::string& dev_name,
                                            const std::string& attr_name,
                                            Tango::EventType evt_type)
{
  int evt_id = -1;

  //- remove event info from our repository
  {
    yat::MutexLock guard(this->subscribed_events_lock);
    
    if ( ! this->event_subscribed_i(dev_name, attr_name, evt_type, &evt_id) )
    {
      THROW_DEVFAILED("EVENT_UNSUBSCRIPTION_ERROR",
                      "failed to unsubscribe to the specified Tango event [no such event]",
                      "EventsConsumerTask::unsubscribe_event");
    }

    EventInfoMapIterator it = this->subscribed_events.find(evt_id);
    if ( it != this->subscribed_events.end() )
    {
      this->subscribed_events.erase(it);
    }
  }
  
  try
  {
    DeviceProxyMapIterator it = this->proxies.find(dev_name);
    if ( it == this->proxies.end() )
    {
      //- this should never happen
      return; 
    }

    //- unsubscribe 
    it->second.proxy->unsubscribe_event(evt_id);

    if ( ! this->proxies[dev_name].dec_usage_counter() )
    {
      this->proxies[dev_name].release();
      this->proxies.erase(dev_name);
    }
  }
  catch (Tango::DevFailed& e)
  {
    RETHROW_DEVFAILED(e,
                      "EVENT_SUBSCRIPTION_ERROR",
                      "failed to subscribe to the specified Tango event [Tango error caught]",
                      "EventsConsumerTask::subscribe_event");
  }
  catch (...)
  {
    THROW_DEVFAILED("EVENT_SUBSCRIPTION_ERROR",
                    "failed to subscribe to the specified Tango event [unknown error caught]",
                    "EventsConsumerTask::unsubscribe_event");
  }
}

// ======================================================================
// EventsConsumerTask::unsubscribe_event
// ======================================================================
void EventsConsumerTask::unsubscribe_event (int evt_id)
{
  yat::MutexLock guard(this->subscribed_events_lock);

  this->unsubscribe_event_i(evt_id, true);
}

// ======================================================================
// EventsConsumerTask::unsubscribe_event
// ======================================================================
void EventsConsumerTask::unsubscribe_event_i (int evt_id, bool erase)
{
  //- remove event info from our repository
  EventInfoMapIterator it = this->subscribed_events.find(evt_id);
  if ( it == this->subscribed_events.end() )
  {
    THROW_DEVFAILED("EVENT_UNSUBSCRIPTION_ERROR",
                    "failed to unsubscribe to the specified Tango event [no such event]",
                    "EventsConsumerTask::unsubscribe_event");
  }

  std::string dev_name = it->second.dev_name;

  if ( erase )
  {
    this->subscribed_events.erase(it);
  }

  try
  {
    DeviceProxyMapIterator it = this->proxies.find(dev_name);
    if ( it == this->proxies.end() )
    {
      //- this should never happen
      return;
    }

    //- unsubscribe
    it->second.proxy->unsubscribe_event(evt_id);

    if ( ! this->proxies[dev_name].dec_usage_counter() )
    {
      this->proxies[dev_name].release();
      this->proxies.erase(dev_name);
    }
  }
  catch (Tango::DevFailed& e)
  {
    RETHROW_DEVFAILED(e,
                      "EVENT_SUBSCRIPTION_ERROR",
                      "failed to subscribe to the specified Tango event [Tango error caught]",
                      "EventsConsumerTask::subscribe_event");
  }
  catch (...)
  {
    THROW_DEVFAILED("EVENT_SUBSCRIPTION_ERROR",
                    "failed to subscribe to the specified Tango event [unknown error caught]",
                    "EventsConsumerTask::unsubscribe_event");
  }
}

// ======================================================================
// EventsConsumerTask::unsubscribe_events
// ======================================================================
void EventsConsumerTask::unsubscribe_events ()
{
  yat::MutexLock guard(this->subscribed_events_lock);

  if ( ! this->subscribed_events.empty() )
  {
    EventInfoMapIterator it  = this->subscribed_events.begin();
    EventInfoMapIterator end = this->subscribed_events.end();

    for ( ; it != end; ++it )
    {
      this->unsubscribe_event_i(it->second.evt_id, false);
    }
    
    this->subscribed_events.clear();
  }
}

#undef _LOCAL_TRACES_

} // namespace


#endif //- #if (TANGO_VERSION_MAJOR >= 8)

