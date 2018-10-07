#pragma once 

#include <iostream>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>
#include <list>
#include "RefCount.h"
#include "MemoryManager.h"

using namespace std;




//noly surpport int key in hash
template<class T>
struct HashLinkNode
{
	HashLinkNode() :
		prev(nullptr), next(nullptr), key(-1), data(nullptr)
	{}
	HashLinkNode* prev;
	HashLinkNode* next;
	int key;
	TRefCountPtr<const T> data;
};

//_head->pre points to the tail
template<class T>
class HashLink
{
private:
	void cache_node(HashLinkNode<T>* node)
	{
		//insert front
		node->next = cache_link_node;
		cache_link_node = node;
	}
	HashLinkNode<T>* get_empty_node()
	{
		if (!cache_link_node)
		{
      allocated++;
      auto* r = new HashLinkNode<T>();
      //printf("a:%d->%d\n", this, r);
			return r;
		}
		auto* ret = cache_link_node;
		cache_link_node = cache_link_node->next;
		return ret;
	}
	void insert_link_front(HashLinkNode<T>* node)
	{
		if (_head->next == node)
		{
			return;
		}
		if (_head->next)
		{
			_head->next->prev = node;
			node->next = _head->next;
			node->prev = _head;
			_head->next = node;
		}
		else
		{
			_head->next = node;
			node->prev = _head;
			node->next = nullptr;
			_head->prev = node;
		}
	}
	void remove_link(HashLinkNode<T>* node)
	{
		auto* next = node->next;
		auto* prev = node->prev;
		node->next = nullptr;
		node->prev = nullptr;
		if (next == nullptr)
		{
			_head->prev = prev;
			prev->next = nullptr;
			return;
		}
		next->prev = prev;
		prev->next = next;
	}

public:
	void insert(TRefCountPtr<const T> a)
	{
		HashLinkNode<T>* node = get_empty_node();
		node->data = a;
		node->key = a->key;
		insert_link_front(node);
		if (_hash_table.size() < (size_t)(a->key + 1))
			_hash_table.resize(a->key * 2 + 2, nullptr);
		_hash_table[node->key] = node;
	}
	void remove(TRefCountPtr<const T> a)
	{
		if (_hash_table.size() < (size_t)(a->key + 1))
			return;
		HashLinkNode<T>* node = _hash_table[a->key];
		if (!node)
			return;
		assert(node->key == a->key);
		remove_link(node);
		cache_node(node);
		_hash_table[a->key] = nullptr;
	}
	HashLinkNode<T>* head() const
	{
		return _head->next;
	}
	HashLinkNode<T>* end() const
	{
		return _head;
	}
	HashLinkNode<T>* next(HashLinkNode<T>* cur) const
	{
		if (!cur)
			return nullptr;
		return cur->next;
	}
	HashLinkNode<T>* prev(HashLinkNode<T>* cur) const
	{
		if (!cur)
			return nullptr;
		return cur->prev;
	}

	bool empty()
	{
		return !_head->next;
	}

	HashLink()
	{
		//mem!
    allocated++;
		_head = new HashLinkNode<T>();
    //printf("a:%d->%d\n",this,_head);
		_head->prev = _head;
		cache_link_node = nullptr;
	}
	~HashLink()
	{
    
		//release link
		HashLinkNode<T>* temp = nullptr;
    
		for (HashLinkNode<T>* l = _head; l != nullptr;)
		{
			temp = l->next;
      //printf("d:%d->%d\n", this, l);
			delete l;
			l = temp;
      
      allocated--;
		}
    
    

		for (HashLinkNode<T>* l = cache_link_node; l != nullptr;)
		{
			temp = l->next;
			delete l;
			l = temp;
      allocated--;
		}
    if (allocated != 0)
      printf("leak\n");
    _hash_table.clear();
    
	}

private:
	HashLinkNode<T>* _head;
	std::vector<HashLinkNode<T>*> _hash_table;

	HashLinkNode<T>* cache_link_node;

  int allocated = 0;
};

enum class EventType
{
	E_UI,
};

class TypeInfo
{
public:
	TypeInfo(const char* type_name, const TypeInfo* base_type_info)
		:_type(get_string_hash(type_name)), _base_type_info(base_type_info)
	{}
	~TypeInfo()
	{}

	bool is_type_of(string_hash type) const
	{
		const TypeInfo* cur = this;
		while (cur)
		{
			if (cur->get_type() == type)
				return true;
			cur = cur->get_base_type_info();
		}
		return false;
	}
	bool is_type_of(const TypeInfo* type_info) const
	{
		const TypeInfo* cur = this;
		while (cur)
		{
			if (cur == type_info)
				return true;
			cur = cur->get_base_type_info();
		}
		return false;
	}

	template<class T>
	bool is_type_of() const
	{
		return is_type_of(T::get_type_info_static());
	}

	string_hash get_type() const
	{
		return _type;
	}

	const std::string& get_type_name() const
	{
		return _type_name;
	}

	const TypeInfo* get_base_type_info() const
	{
		return _base_type_info;
	}

private:
	string_hash _type;
	std::string _type_name;
	const TypeInfo* _base_type_info;
};

class WIPEvent
{
public:

};

class WIPObject;

class EventHandlerBase
{
public:
	virtual void set_sender_and_event_type(WIPObject* sender, string_hash event_type) = 0;

	virtual void call(void* event_params) = 0;

	virtual EventHandlerBase* clone() const = 0;

	virtual WIPObject* get_receiver() const = 0;

	virtual WIPObject* get_sender()  const = 0;

	virtual string_hash get_event_type() const = 0;

	virtual void* get_user_data() const = 0;
};

template <class T>
class EventHandler : public EventHandlerBase
{
public:
	typedef void (T::*handler_function_t)(string_hash, void*);

	EventHandler(T* receiver, handler_function_t func, void* user_data = 0) :
		_receiver(receiver), _sender(nullptr), _data(user_data), _function(func)
	{
	}
	~EventHandler(){}

	virtual void set_sender_and_event_type(WIPObject* sender, string_hash event_type) override
	{
		_sender = sender;
		_event_type = event_type;
	}

	virtual void call(void* event_params) override
	{
		T* receiver = static_cast<T*>(_receiver);
		(receiver->*_function)(_event_type, event_params);
	}

	virtual EventHandlerBase* clone() const override
	{
		return new EventHandler(static_cast<T*>(_receiver), _function, _data);
	}

	virtual WIPObject* get_receiver() const override { return _receiver; }

	virtual WIPObject* get_sender() const override { return _sender; }

	virtual string_hash get_event_type() const override { return _event_type; }

	virtual void* get_user_data() const override { return _data; }

private:
	WIPObject* _receiver;
	WIPObject* _sender;
	string_hash _event_type;
	void* _data;
	handler_function_t _function;
};

//noly use in receiver class
#define WIP_EVENT_HANDLER(class_name,function) (new EventHandler<class_name>(this,&class_name::function))
#define WIP_EVENT_HANDLER_USERDATA(class_name,function,user_data) (new EventHandler<class_name>(this,&class_name::function,user_data))
//out of class
#define WIP_EVENT_HANDLER_OUT(class_name,function,receiver) (new EventHandler<class_name>(receiver,&class_name::function))
#define WIP_EVENT_HANDLER_USERDATA_OUT(class_name,function,user_data,receiver) (new EventHandler<class_name>(receiver,&class_name::function,user_data))


#define WIPOBJECT(type_name,basetype_name) \
	public: \
		typedef type_name class_name; \
		typedef basetype_name baseclass_name; \
    virtual string_hash get_type() const {return get_type_info_static()->get_type();} \
    virtual const std::string&  get_type_name() const {return get_type_info_static()->get_type_name();} \
    virtual const TypeInfo* get_type_info() const {return get_type_info_static();} \
    static string_hash get_type_static() {return get_type_info_static()->get_type();} \
    static const std::string& get_type_name_static() {return get_type_info_static()->get_type_name();} \
    static const TypeInfo* get_type_info_static() {static const TypeInfo type_info_static(#type_name,basetype_name::get_type_info_static()); return &type_info_static;} \




class WIPSprite;
class WIPTickComponent;

#define WIPTICKCOMPONENT(type_name)\
	public:\
	static WIPTickComponent* create_tick_component(WIPSprite* host){return new type_name(host);}\
	static void register_tick_component(){WIPObject::_tick_component_register_tb[get_string_hash(#type_name)]=type_name::create_tick_component;}\

class WIPObject : public FRefCountedObject
{
public:
	static void* mem_header;
	virtual ~WIPObject()
	{
		unsubscribe_all_events();
	}
	WIPObject()
	{

	}
	virtual string_hash get_type() const = 0;
	virtual const std::string&  get_type_name() const = 0;
	virtual const TypeInfo* get_type_info() const = 0;


	//make base class type infomation
	static const TypeInfo* get_type_info_static() { return 0; }

	bool is_type_of(string_hash type) const
	{
		return get_type_info()->is_type_of(type);
	}
	bool is_type_of(const TypeInfo* type_info) const
	{
		return get_type_info()->is_type_of(type_info);
	}
	template<class T> bool is_type_of() const { return is_type_of(T::get_type_info_static()); }

	virtual void handle_event(WIPObject* sender, string_hash event_type, void* data);


	//void send_event(const WIPObject* sender,WIPEvent evt);
	void subscribe_event(string_hash evt_tp, EventHandlerBase* handler, int priority = -1);
	void subscribe_event(WIPObject* sender, string_hash evt_tp, EventHandlerBase* handler, int priority = -1);

	void unsubscribe_event(string_hash event_type);
	void unsubscribe_event(WIPObject* sender, string_hash event_type);
	void unsubscribe_events(WIPObject* sender);
	void unsubscribe_all_events();
	void unsubscribe_all_events_except(const std::vector<string_hash>& exceptions, bool only_user_data){}

	void send_event(string_hash event_type)
	{
		send_event(event_type, nullptr);
	}

	void send_event(string_hash event_type, void* event_data);


	void remove_event_sender(WIPObject* sender);

	void* get_event_data() const;

	WIPObject* get_event_sender() const;
	EventHandlerBase* get_event_handler() const;

	bool has_subscribed_event(string_hash event_type) const;
	bool has_subscribed_event(WIPObject* sender, string_hash event_type) const;

	bool has_event_handlers() const { return !_event_handlers.empty(); }

	static WIPTickComponent* create_tick_component(const char* cname,WIPSprite* host);
private:
	/// Find the first event handler with no specific sender.
	std::list<EventHandlerBase*>::const_iterator _find_event_handler(string_hash event_type, EventHandlerBase** previous = 0) const
	{
		std::list<EventHandlerBase*>::const_iterator it = _event_handlers.begin();
		for (; it != _event_handlers.end(); ++it)
		{
			if ((*it)->get_event_type() == event_type)
				return it;
		}
		return _event_handlers.end();
	}
	/// Find the first event handler with specific sender.
	std::list<EventHandlerBase*>::const_iterator _find_specific_event_handler(WIPObject* sender, EventHandlerBase** previous = 0) const
	{
		std::list<EventHandlerBase*>::const_iterator it = _event_handlers.begin();
		for (; it != _event_handlers.end(); ++it)
		{

			if ((*it)->get_sender() == sender)
				return it;
		}
		return _event_handlers.end();
	}
	/// Find the first event handler with specific sender and event type.
	std::list<EventHandlerBase*>::const_iterator _find_specific_event_handler(WIPObject* sender, string_hash event_type, EventHandlerBase** previous = 0) const
	{
		std::list<EventHandlerBase*>::const_iterator it = _event_handlers.begin();
		for (; it != _event_handlers.end(); ++it)
		{
			if ((*it)->get_sender() == sender && (*it)->get_event_type() == event_type)
				return it;
		}
		return _event_handlers.end();
	}
	/// Remove event handlers related to a specific sender.
	void _remove_event_sender(WIPObject* sender);

	std::list<EventHandlerBase*> _event_handlers;
	typedef std::list<EventHandlerBase*> event_list_t;

protected:
	//sprite creator not use
	static std::map<string_hash, WIPSprite*(*)(WIPSprite*)> _;
	//tick component creator : name-creator
	static std::map<string_hash, WIPTickComponent*(*)(WIPSprite*)> _tick_component_register_tb;
public:
	void static_init();
};


