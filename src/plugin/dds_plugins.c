#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "dict.h"
#include "dds_plugins.h"




// ---------------

static int well_formed(const char *list, int *count){
	
	#define outside(var, s) do{var = (var && s[0]!=',' && s[strlen(s)-1]!=',');}while(0);
	int good = 1;
	//outside(good, list)
	char* cur_split = strtok(list, ",");
	while(cur_split){
		//outside(good, cur_split)
		(*count)++;
		cur_split = strtok(NULL, ",");
	}	
	#undef outside
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
	target->instance = lua_open();
	luaL_openlibs(target->instance);
	TEST(!luaL_loadfile(target->instance, plugin_file), "Couldn't load file for a plugin...");
	TEST(!lua_pcall(target->instance, 0, 0, 0), "Couldn't run plugin file...");
	lua_getglobal(target->instance, "name");
	printf("Attempting to print...\n");
	TEST(!lua_pcall(target->instance, 0, 0, 0), "Couldn't call name...");
	printf("Done printing...\n");
	return 1;
		
}
lua_plugin_list *init_plugins(Dict *config){
	printf("Initializing plugins...\n");
	static char once = 0;
	ASSERT(once++ == 0, "init_plugins was run more then once...") //We only run this once
	printf("After assert\n");
	lua_plugin_list plugins;
	printf("After plugin list...\n");
	dump_dict(config);
	Dict* val = dict_get_val(config, "plugins");	
	printf("After val");
	char* plugins_list_str = val->value;


	int plugin_count = 0;
	if(!well_formed(plugins_list_str, &plugin_count)){
		printf("Inside wellformed...\n");
		PERR("Bad plugin list in config file...")
	}
	
	plugins.num_plugins = plugin_count;
	plugins.list = malloc(sizeof(lua_plugin) * plugin_count);
	lua_plugin *iter = plugins.list;


	char* cur_plugin;
	cur_plugin = strtok(plugins_list_str, ",");
	while(cur_plugin){
		char* file_name = path_to_plugin(cur_plugin);	
		TEST(init_plugin(cur_plugin, iter, file_name), "Failed to initialize plugin");
		free(file_name);

		cur_plugin = strtok(NULL, ",");
		iter++;
	}
	printf("Done loading plugins...\n");
}

