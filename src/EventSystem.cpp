#include "EventSystem.h"
#include "Object.h"
#include <algorithm>

bool sort_g(const EventReceivers::EventReceiver& a, const EventReceivers::EventReceiver& b)
{
  return a.priority > b.priority;
}

EventReceivers::EventReceivers(){}
void EventReceivers::add_receiver(WIPObject* object,int priority)
{
  if (!object)
    return;
   receivers_.push_back(EventReceiver(object,priority));
   std::sort(receivers_.begin(), receivers_.end(), sort_g);
}

void EventReceivers::remove_receiver(WIPObject* object)
{

}

EventManager* EventManager::get_instance()
{
  static EventManager* _instance = nullptr;
  if (!_instance)
    _instance = new EventManager();
  return _instance;
}

void EventManager::add_event_receiver(WIPObject* receiver, string_hash event_type, int priority)
{
  auto ret = no_sender_event_receivers_.find(event_type);
  if (ret == no_sender_event_receivers_.end())
  {
    no_sender_event_receivers_[event_type] = new EventReceivers();
  }
  no_sender_event_receivers_[event_type]->add_receiver(receiver,priority);
}

void EventManager::add_event_receiver(WIPObject* receiver, WIPObject* sender, string_hash event_type, int priority)
{
  auto ret = sender_event_receivers_.find(sender);
  if (ret == sender_event_receivers_.end())
  {
    sender_event_receivers_[sender] = std::map<string_hash, EventReceivers*>();
    sender_event_receivers_[sender][event_type] = new EventReceivers();
  }
  else
  {
    auto ret1 = sender_event_receivers_[sender].find(event_type);
    if (ret1 == sender_event_receivers_[sender].end())
    {
      sender_event_receivers_[sender][event_type] = new EventReceivers();
    }
  }
  sender_event_receivers_[sender][event_type]->add_receiver(receiver,priority);
}