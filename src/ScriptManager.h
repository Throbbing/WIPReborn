#pragma once
#include "../libs/lua523/src/lua.hpp"
#include "RefCount.h"

//note : do compile lua code as c code!
class WIPScriptManager : public FRefCountedObject
{
public:
	static WIPScriptManager* instance();

	WIPScriptManager();
	~WIPScriptManager();

	bool startup();
	void shutdown();
	bool load_file(const char* file);
	bool do_string(const char* file);
	void remove_global_var(const char* name);
	void call_table_function(const char* tname, const char* fname, const char* param_types, ...);
	void call(const char* function);
	void call(const char* function,const char* param_types,...);

protected:

private:
	void generate_api();
	static WIPScriptManager* _instance;
	
	lua_State *_L;
};

extern WIPScriptManager* g_script_manager;