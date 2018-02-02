#pragma once
#include <map>
#include <string>
#include "RefCount.h"
#include "MemoryPool.h"
#include "Logger.h"

//refcount is used for both system new and wip_new
//if class use both refcount and wip_new, be sure to overload the 'delete' operater for it.

typedef unsigned int string_hash;

inline unsigned SDBMHash(unsigned hash, unsigned char c);

unsigned get_string_hash(const char* str);

extern RBPoolAllctor* g_pool_allocator;

class WIPMemoryManager 
{
public:
	void report()
	{

		{
			g_pool_allocator->report();
		}

	}
	void summary()
	{
		LOG_NOTE("allocated : %d", allocated); 
	}

	int allocated=0;
	std::map<std::string, void*> pools;
};

extern WIPMemoryManager* g_mem_manager;

#define WIP_NEW_POOL(classname)\
 if(g_mem_manager->pools.find(#classname)==g_mem_manager->pools.end())\
          		{\
	g_pool_allocator->new_pool(&g_mem_manager->pools[#classname],sizeof(classname),#classname,get_string_hash(#classname));\
          	}

#define WIP_NEW(classname) \
	g_pool_allocator->alloc(&g_mem_manager->pools[#classname],sizeof(classname))

#define WIP_DELETE(ptr,classname)\
	g_mem_manager->allocated--;\
	g_pool_allocator->free(&g_mem_manager->pools[#classname],ptr,sizeof(classname))

#define WIP_MEM(classname)\
	static void init_mem(){\
	WIP_NEW_POOL(classname)\
	}\
	void* operator new(size_t size){\
	static struct _{_(){init_mem();}} _; \
	void* p = g_pool_allocator->alloc(&g_mem_manager->pools[#classname],size);\
	g_mem_manager->allocated++;\
	printf("alloc [%s] : %X | %d\n",#classname,p,g_mem_manager->allocated);\
	return p;\
	}\
	void operator delete(void *p){\
	if (!p)return;\
	printf("delete [%s]: %X | %d\n",#classname,p,g_mem_manager->allocated); \
	WIP_DELETE(p,classname);\
	}