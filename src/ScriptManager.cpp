#include "ScriptManager.h"
#include "LuaAPI.h"
#include <string>
#include "IniHelper.h"

WIPScriptManager* WIPScriptManager::_instance = 0;

WIPScriptManager* WIPScriptManager::instance()
{
	if(!_instance)
		_instance = new WIPScriptManager();
	return _instance;
}

WIPScriptManager::WIPScriptManager()
{

}

WIPScriptManager::~WIPScriptManager()
{
	delete _instance;
}

bool WIPScriptManager::startup()
{
	_L = luaL_newstate();
	luaL_openlibs(_L);
	generate_api();
	return true;
}

void WIPScriptManager::shutdown()
{
	lua_close(_L);
	_L = 0;
}

bool WIPScriptManager::load_file(const char* file)
{
	std::string file_path = "../LuaSyatem/OG/init.lua";
	std::string path = "WIPCFG.ini";
	if(WIPIniHelper::reset_ini_file(path.data()))
	{
		WIPIniHelper::get_string("Common","script_init",file_path);
	}
	else
	{
		printf("Read ini failed!\n");
		getchar();
		exit(0);
	}
	WIPIniHelper::close();
	if(luaL_dofile(_L,file_path.c_str()))
	{
		printf("load file %s error: %s\n", file, lua_tostring(_L, -1));
		getchar();
	}
	
	/*
	*We use lua_pcall to run this chunk(Lua regard this file as 
	*a function also called chunk) so that all the globals will be available. 
	*/
	if (luaL_loadfile(_L, file) || lua_pcall(_L, 0, 0, 0))
	{
		printf("load file %s error: %s\n", file, lua_tostring(_L, -1));
		getchar();
	}
	return true;
}

void WIPScriptManager::call(const char* function)
{
	lua_getglobal(_L,function);
	if(1==lua_isnil(_L,-1))
	{
		printf("There is no function called \"%s\" \n",function);
		return;
	}
	if(lua_pcall(_L,0,0,0)!=0)
		printf("In function \"%s\" : %s\n",function,lua_tostring(_L,-1));
}

void WIPScriptManager::call(const char* function,const char* param_types,...)
{
	//only copy from without seeing
	lua_getglobal(_L, function);
	if (lua_isnil(_L, -1) == 1)
	{
		printf("There is no function called \"%s\" \n",function);
		return;
	}

	va_list vl;
	va_start(vl, param_types);
	int arg_num = 0;
	while (*param_types)
	{
		switch (*param_types++)
		{
		case 'd':
			lua_pushnumber(_L, va_arg(vl, double));
			break;
		case 'i':
			lua_pushnumber(_L, va_arg(vl, int));
			break;
		case 's':
			lua_pushstring(_L, va_arg(vl, char*));
			break;
		default:
			printf("invalid param type...");
			break;
		}
		luaL_checkstack(_L, 1, "too many params...");
		arg_num++;
	}
	va_end(vl);
	if (lua_pcall(_L, arg_num, 0, 0) != 0)
		printf("In function \"%s\" : %s\n",function,lua_tostring(_L,-1));
}


void WIPScriptManager::generate_api()
{

	
	 luaL_requiref(_L, "lib", L_load_lib, 0);
	//luaL_setfuncs(_L,lib,1);
}



WIPScriptManager* g_script_manager = WIPScriptManager::instance();