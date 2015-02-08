#ifndef DDS_LUA
#define DDS_LUA

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
#include "../dict.h"

#define TEST(expr, str) ASSERT(expr, str)
#define ASSERT(expr, str) do{if(!expr){PERR(str);}}while(0);
#define PERR(str) do{fprintf(stderr, "Error: %s\n", str);exit(1);}while(0);
typedef struct _dds_lua_plugin{
	char name[64];
	lua_State *instance;
}lua_plugin;

typedef struct _lua_plugin_list{
	int num_plugins;
	lua_plugin *list;
}lua_plugin_list;
lua_plugin_list *init_plugins(Dict *config);
#endif /* DDS_LUA */
