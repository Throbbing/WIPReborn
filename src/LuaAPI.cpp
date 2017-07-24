#include "ResourceManager.h"
#include "Sprite.h"
#include "Logger.h"

#include "LuaAPI.h"
#include "LuaUI.h"


void _check_parameters(lua_State* L,int n)
{
	int s = lua_gettop(L);
	if(s<n)
	{
		printf("function need %d,there is %d\n",n,s);
		getchar();
		exit(0);
	}
}

/*LuaAPI
 *note LuaAPI only support the function like this:
 *int function(lua_state*);
 *called by generate_api()
 *
 */
int L_load_lib(lua_State* L)
{
	luaL_Reg lib[] = 
	{
		{"init",L_init},
		{NULL,NULL}
	};
	luaL_newlib(L, lib);
	return 1;
}

int L_init(lua_State* L)
{
	/*
	bool ret = false;
	if(g_res_manager->startup())
	{
	ret = true;
	}
	else
	{
		ret = false;
	}
	lua_pushboolean(L,1);
	*/
	printf("lua\n");
	return 0;
}