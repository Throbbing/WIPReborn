#pragma once
#include "../libs/lua523/src/lua.hpp"

void _check_parameters(lua_State* L,int n);

int L_load_lib(lua_State* L);


int L_init(lua_State* L);