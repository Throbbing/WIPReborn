#pragma once
#include "Object.h"
#include <map>
#include <vector>
#include "RefCount.h"

class WIPObject;

class EventReceivers
{
public:
  struct EventReceiver
  {
    EventReceiver(WIPObject* o, int prior):
      priority(prior), object(o){}
    int priority;
    WIPObject* object;
  };
  EventReceivers();
  void add_receiver(WIPObject* object,int priority=-1);
  void remove_receiver(WIPObject* object);
  std::vector<EventReceiver> receivers_;

};

class EventManager : public FRefCountedObject
{
public:
  static EventManager* get_instance();
  //no sender events
  std::map<string_hash, EventReceivers*> no_sender_event_receivers_;
  std::map<WIPObject*, std::map<string_hash, EventReceivers*> > sender_event_receivers_;
  std::vector<WIPObject*> senders_;
  //add receiver who receives no sender events
  void add_event_receiver(WIPObject* receiver, string_hash event_type, int priority=-1);
  void add_event_receiver(WIPObject* receiver, WIPObject* sender, string_hash event_type, int priority=-1);
  void remove_event_sender(WIPObject* sender);
  void remove_event_receiver(WIPObject* receiver, WIPObject* sender, string_hash eventType);
  void remove_event_receiver(WIPObject* receiver, string_hash eventType);
  /// Return event receivers for a sender and event type, or null if they do not exist.
  EventReceivers* get_event_receivers(WIPObject* sender, string_hash eventType)
  {
    map<WIPObject*, map<string_hash, EventReceivers*> >::iterator i = sender_event_receivers_.find(sender);
    if (i != sender_event_receivers_.end())
    {
      map<string_hash, EventReceivers* >::iterator j = i->second.find(eventType);
      return j != i->second.end() ? j->second : nullptr;
    }
    else
      return nullptr;
  }

  /// Return event receivers for an event type, or null if they do not exist.
  EventReceivers* get_event_receivers(string_hash eventType)
  {
    map<string_hash, EventReceivers*>::iterator i = no_sender_event_receivers_.find(eventType);
    return i != no_sender_event_receivers_.end() ? i->second : nullptr;
  }
};