#include <stdio.h>
#include <stdlib.h>
#include "dds_plugins.h"
#include "dds_sem.h"
#include "dds_plugin_api.h"
#include "dds_io.h"
static int send_message(lua_State*);
static int lock_msg_queue(lua_State*);
static int unlock_msg_queue(lua_State*);
void init_api(lua_plugin *p){
	lua_pushcfunction(p->instance, send_message);
	lua_setglobal(p->instance, "send_message");
	lua_pushcfunction(p->instance, lock_msg_queue);
	lua_setglobal(p->instance, "lock_msg_queue");
	lua_pushcfunction(p->instance, unlock_msg_queue);
	lua_setglobal(p->instance, "unlock_msg_queue");
	dds_sem s = dds_open_sem(p->name, 1);
	int val = (int)s;
	lua_pushnumber(p->instance, val);
	lua_setglobal(p->instance, "my_name");
}
static int send_message(lua_State *L){
	if(!lua_isstring(L, 1)){
		//TODO fail horribly
	}
	const char *lua_str = lua_tostring(L, -1);
	int len = lua_strlen(L, -1);
	char* ours = malloc(len + 1);
	if(lua_str[len] != '\0'){
		//TODO crash....
	}
	strcpy(ours, lua_str);
	write_to_pipe(ours);
	//TODO might break
	free(ours);
	return 0;
}

static int lock_msg_queue(lua_State *L){
	lua_getglobal(L, "my_sem");
	dds_sem sem = (dds_sem)(int)lua_tonumber(L, -1); //This will surely be a warning idgaf
	if(try_dds_sem(sem) != -1){
		printf("Couldn't lock sem\n");

	}
	return 0;	
}
static int unlock_msg_queue(lua_State *L){
	lua_getglobal(L, "my_sem");
	dds_sem sem = (dds_sem)(int)lua_tonumber(L, -1);
	if(!release_dds_sem(sem)){
		printf("Couldn't release sem...\n");

	}
	return 0;
}

void add_msg(lua_plugin *p, char* msg){
	lock_msg_queue(p->instance);
	lua_getglobal(p->instance, "addMessage");
	lua_pushstring(p->instance, msg);
	lua_pcall(p->instance, 1, 0, 0);
	unlock_msg_queue(p->instance);
}
