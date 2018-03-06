#include "Object.h"
#include "EventSystem.h"
#include "MemoryManager.h"

WIPMemoryManager* g_mem_manager = new WIPMemoryManager();



inline unsigned SDBMHash(unsigned hash, unsigned char c)
{
  return c + (hash << 6) + (hash << 16) - hash;
}

unsigned get_string_hash(const char* str)
{
  unsigned hash = 0;

  if (!str)
    return hash;

  while (*str)
  {
    // Perform the actual hashing as case-insensitive
    char c = *str;
    hash = SDBMHash(hash, (unsigned char)std::tolower(c));
    ++str;
  }

  return hash;
}

void WIPObject::subscribe_event(string_hash evt_tp, EventHandlerBase* handler, int priority)
{
  if (!handler)
    return;
  handler->set_sender_and_event_type(nullptr, evt_tp);
  event_list_t::const_iterator old = _find_specific_event_handler(nullptr, evt_tp, 0);
  if (old != _event_handlers.end())
  {
    _event_handlers.erase(old);
    _event_handlers.push_front(handler);
  }
  else
  {
    _event_handlers.push_front(handler);
    EventManager::get_instance()->add_event_receiver(this, evt_tp,priority);
  }

}

void WIPObject::subscribe_event(WIPObject* sender, string_hash evt_tp, EventHandlerBase* handler, int priority)
{
  if (!sender || !handler)
  {
    delete handler;
    return;
  }

  handler->set_sender_and_event_type(sender, evt_tp);
  event_list_t::const_iterator old = _find_specific_event_handler(sender, evt_tp, 0);
  if (old != _event_handlers.end())
  {
    _event_handlers.erase(old);
    _event_handlers.push_front(handler);
  }
  else
  {
    _event_handlers.push_front(handler);
    EventManager::get_instance()->add_event_receiver(this, sender, evt_tp,priority);
  }
}

void WIPObject::remove_event_sender(WIPObject* sender)
{
	event_list_t::iterator handler = _event_handlers.begin();
	//EventHandlerBase* prev;
	while (handler!=_event_handlers.end())
	{
		if ((*handler)->get_sender() == sender)
		{
			handler = _event_handlers.erase(handler);
		}
	}
}

void WIPObject::unsubscribe_event(string_hash event_type)
{
	while (1)
	{
		EventHandlerBase* prev;
		event_list_t::const_iterator handler = _find_event_handler(event_type, &prev);
		if (handler != _event_handlers.end())
		{
			if ((*handler)->get_sender())
			{
				EventManager::get_instance()->remove_event_receiver(this, (*handler)->get_sender(), event_type);
			}
			else
			{
				EventManager::get_instance()->remove_event_receiver(this, event_type);
			}
			_event_handlers.erase(handler);
		}
		else
			break;

	}
}

void WIPObject::unsubscribe_event(WIPObject* sender, string_hash event_type)
{
	if (!sender)
		return;
	EventHandlerBase* evp;
	event_list_t::const_iterator handler = _find_specific_event_handler(sender, event_type, &evp);
	if (handler != _event_handlers.end())
	{
		EventManager::get_instance()->remove_event_receiver(this, (*handler)->get_sender(), event_type);
		_event_handlers.erase(handler);
	}
}

void WIPObject::unsubscribe_events(WIPObject* sender)
{
	if (!sender)
		return;
	while (1)
	{
		EventHandlerBase* prev;
		event_list_t::const_iterator handler = _find_specific_event_handler(sender, &prev);
		if (handler != _event_handlers.end())
		{

			EventManager::get_instance()->remove_event_receiver(this, (*handler)->get_sender(), (*handler)->get_event_type());
			_event_handlers.erase(handler);
		}
		else
			break;

	}
}

void WIPObject::unsubscribe_all_events()
{
	while (1)
	{
		event_list_t::const_iterator handler = _event_handlers.begin();
		if (handler != _event_handlers.end())
		{
			if ((*handler)->get_sender())
			{
				EventManager::get_instance()->remove_event_receiver(this, (*handler)->get_sender(), (*handler)->get_event_type());
			}
			else
			{
				EventManager::get_instance()->remove_event_receiver(this, (*handler)->get_event_type());
			}
			_event_handlers.erase(handler);
		}
		else
			break;

	}
}

void WIPObject::send_event(string_hash event_type, void* event_data)
{
  EventReceivers* group = EventManager::get_instance()->get_event_receivers(this, event_type);
  std::map<WIPObject*, WIPObject*> processed;
  if (group)
  {
    size_t size = group->receivers_.size();
    for (size_t i = 0; i < size; ++i)
    {
      WIPObject* rec = group->receivers_[i].object;
      // Holes may exist if receivers removed during send
      if (!rec)
        continue;
      rec->handle_event(this, event_type, event_data);

      processed[rec] = rec;
    }
  }

  group = EventManager::get_instance()->get_event_receivers(event_type);
  if (group)
  {
    if (processed.empty())
    {
      size_t size = group->receivers_.size();
      for (size_t i = 0; i < size; ++i)
      {
        WIPObject* rec = group->receivers_[i].object;
        // Holes may exist if receivers removed during send
        if (!rec)
          continue;
        rec->handle_event(this, event_type, event_data);
      }
    }
    else
    {
      size_t size = group->receivers_.size();
      for (size_t i = 0; i < size; ++i)
      {
        WIPObject* rec = group->receivers_[i].object;

        if (!rec || processed.find(rec) != processed.end())
        {
          continue;
        }

        rec->handle_event(this, event_type, event_data);
      }

    }
  }

}


void WIPObject::handle_event(WIPObject* sender, string_hash event_type, void* data)
{
  EventHandlerBase* hassender = nullptr, *nosender = nullptr;
  event_list_t::iterator it = _event_handlers.begin();
  //只可能有一个处理此类发送者和事件类型的handler
  //无sender指定类型的事件handler也只可能有一个
  for (; it != _event_handlers.end(); ++it)
  {
    if ((*it)->get_event_type() == event_type)
    {
      if (!(*it)->get_sender())
      {
        nosender = (*it);
      }
      else if ((*it)->get_sender() == sender)
      {
        hassender = (*it);
        break;
      }
    }
  }

  if (hassender)
  {
    hassender->call(data);
    return;
  }

  if (nosender)
  {
    nosender->call(data);
  }

}
std::map<string_hash, WIPTickComponent*(*)(WIPSprite*)> WIPObject::_tick_component_register_tb;
WIPTickComponent* WIPObject::create_tick_component(const char* cname, WIPSprite* host)
{
	std::map<string_hash, WIPTickComponent*(*)(WIPSprite*)>::iterator it = _tick_component_register_tb.find(get_string_hash(cname));
	if (it != _tick_component_register_tb.end())
	{
		return it->second(host);
	}
	LOG_ERROR("%s not regist...", cname);
	return nullptr;
}

//mem

RBPoolAllctor* g_pool_allocator = RBPoolAllctor::instance();