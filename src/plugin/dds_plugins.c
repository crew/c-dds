#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include "dds_globals.h"
#include "dict.h"
#include "dds_plugins.h"
#include "dds_plugin_api.h"
#include "dds_sem.h"
#include <pthread.h>

// ---------------
static void stackDump (lua_State *L) {
      int i;
      int top = lua_gettop(L);
      printf("Stack size %d\n", top);
      for (i = 1; i <= top; i++) {  /* repeat for each level */
        int t = lua_type(L, i);
        switch (t) {
    
          case LUA_TSTRING:  /* strings */
            printf("`%s'", lua_tostring(L, i));
            break;
    
          case LUA_TBOOLEAN:  /* booleans */
            printf(lua_toboolean(L, i) ? "true" : "false");
            break;
    
          case LUA_TNUMBER:  /* numbers */
            printf("%g", lua_tonumber(L, i));
            break;
    
          default:  /* other values */
            printf("%s", lua_typename(L, t));
            break;
    
        }
        printf("  ");  /* put a separator */
      }
      printf("\n");  /* end the listing */
    }

static int well_formed(char *list, int *count){
	
	#define outside(var, s) do{var = (var && s[0]!=',' && s[strlen(s)-1]!=',');}while(0);
	int good = 1;
	//outside(good, list)
	char* cur_split = strtok(list, ",");
	while(cur_split){
		//outside(good, cur_split)
		printf("Got %s\n", cur_split);
		(*count)++;
		printf("Count = %d\n", *count);
		cur_split = strtok(NULL, ",");
	}
	#undef outside
	printf("Made it!\n");
	return good;

}
static char* path_to_plugin(const char *name){
	int char_count = strlen(PLUG_FOLDR) + strlen(name) + 1;
	char* str = malloc(char_count);
	*str = '\0';
	strcat(str, PLUG_FOLDR);
	strcat(str, name);
	return str;
}
static int init_plugin(char *name, lua_plugin *target, char* plugin_file){
	target->instance = luaL_newstate();
	luaL_openlibs(target->instance);
	TEST(!luaL_loadfile(target->instance, PLUG_API_LUA), "Couldn't open dds_lua lib");
	TEST(!lua_pcall(target->instance, 0, 0, 0), "Couldn't run dds_lua lib");
	TEST(!luaL_loadfile(target->instance, plugin_file), "Couldn't load file for a plugin...");
	TEST(!lua_pcall(target->instance, 0, 0, 0), "Couldn't run plugin file...");
	init_api(target);
	
	return 1;
		
}
char* get_plugins_list_str(Dict* d){
	if(!d)
		return NULL;
	d = d->next;
	while(d){
		printf("Key %s\n", d->key);
		if(!strcmp(d->key, "plugins")){
			printf("Returning val...\n");
			return (char*)d->value;
		}
		d = d->next;
	}
	printf("No key ...\n");
	return NULL;

}
//All plugins required to have a runPlugin
void *run_plugin(void* args){
	lua_plugin *p = (lua_plugin *)args;
	lua_getglobal(p->instance, "runPlugin");
	if(!lua_pcall(p->instance, 0, 0, 0)){
		printf("Couldn't call runPlugin for plugin %s\n", p->name);
		pthread_exit(NULL);
	}
	//TODO account for non-continuoues plugins
	lua_getglobal(p->instance, "my_sem");
	dds_sem s = (dds_sem)((int)lua_tonumber(p->instance, -1));
	final_close_sem(s);
	lua_close(p->instance);
	pthread_exit(NULL);

}
lua_plugin_list *init_plugins(Dict *config){
	printf("Initializing plugins...\n");
	static char once = 0;
	ASSERT(once++ == 0, "init_plugins was run more then once...") //We only run this once
	lua_plugin_list plugins;
	char* plugins_list_str = get_plugins_list_str(config);

	int plugin_count = 0;
	if(!well_formed(plugins_list_str, &plugin_count)){
		PERR("Bad plugin list in config file...")
	}
	plugins.num_plugins = plugin_count;
	plugins.list = malloc(sizeof(lua_plugin) * plugin_count);
	lua_plugin *iter = plugins.list;


	char* cur_plugin;
	//TODO count tokens...
	cur_plugin = strtok(plugins_list_str, ",");
	while(cur_plugin){
		char* file_name = path_to_plugin(cur_plugin);	
		printf("Got filename %s\n", file_name);
		TEST(init_plugin(cur_plugin, iter, file_name), "Failed to initialize plugin");
		free(file_name);
		pthread_create(&iter->thread, NULL, run_plugin, (void*)iter);
		cur_plugin = strtok(NULL, ",");
		iter++;
	}
	printf("Done loading plugins...\n");
	return &plugins;
}

