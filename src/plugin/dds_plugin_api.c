int send_message(lua_State *L){
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
