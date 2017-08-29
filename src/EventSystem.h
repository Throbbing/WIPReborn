#pragma once
#include "Object.h"
#include <map>
#include <vector>

class WIPObject;

class EventReceivers
{
public:
  EventReceivers();
  void add_receiver(WIPObject* object);
  void remove_receiver(WIPObject* object);
  std::vector<WIPObject*> receivers_;
};

class EventManager
{
public:
  static EventManager* get_instance();
  //no sender events
  std::map<string_hash, EventReceivers*> no_sender_event_receivers_;
  std::map<WIPObject*, std::map<string_hash, EventReceivers*> > sender_event_receivers_;
  std::vector<WIPObject*> senders_;
  //add receiver who receives no sender events
  void add_event_receiver(WIPObject* receiver, string_hash event_type);
  void add_event_receiver(WIPObject* receiver, WIPObject* sender, string_hash event_type);
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