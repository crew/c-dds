#include "dds_plugins.h"
thread_container* make_thread_container(void){
	thread_container* tmp = (thread_container*)malloc(sizeof(thread_container));
	tmp->size = 0;
	tmp->thread_arr = NULL;
	return tmp;
}
void delete_thread_container(thread_container* container){
	int i;
	for(i = 0;i < container->size; i++){
		delete_plugin_thread(container->thread_arr[i]);
	}
	free(container);
}
plugin_thread* make_plugin_thread(char* name){
	plugin_thread* tmp = (plugin_thread*)malloc(sizeof(plugin_thread));
	//I would love to just name[63] = '\0' but i don't want to change the string :/
	if(strlen(name) > 63){
		strncpy(tmp->name, name, 63);
		tmp->name[63] = '\0';
	}else{
		strcpy(tmp->name, name);
	}
	return tmp;
}
void delete_plugin_thread(plugin_thread* thread){
	free(thread);
}
void thread_container_add(thread_container* c, plugin_thread* to_add){
	plugin_thread** new_arr = (plugin_thread**)malloc(sizeof(plugin_thread*) * c->size+1);
	memcpy(new_arr, c->thread_arr, sizeof(plugin_thread*) * c->size);
	new_arr[c->size] = to_add;
	free(c->thread_arr);
	c->thread_arr = new_arr;
	++c->size;	
}

void init_plugin(char* plugin, obj_list container){
	char* myplg = DYN_STR(plugin);
	char* path = (char*)malloc(strlen(plugin)+strlen(PLUGINS_PATH)+1);
	if(isupper(myplg[0])){
		myplg[0] = tolower(myplg[0]);
	}
	strcpy(path, PLUGINS_PATH);
	strcat(path, myplg);
	PyObject* plug_module = PyImport_ImportModule(path);
	if(islower(myplg[0])){
		myplg[0] = toupper(myplg[0]);

	}
	PyObject* clazz = PyObject_GetAttr(plug_module, myplg);
	Py_DECREF(plug_module);
	free(myplg);
	free(path);
	if(!PyClass_Check(clazz)){
		fprintf(stderr, "Got something that wasn't a class object...\n");
	}

	PyObject* plugin_instance = PyInstance_New(clazz, NULL, NULL);
	Py_DECREF(clazz);
	obj_list_add(container, plugin_instance);
	
	
	
	
	
	
	
	/*
	char* from = "from Plugins.";
	char* imp = " import ";
	char* ln = "\n";

	char* plugin_file_name = (char*)malloc(strlen(plugin)+1);
	strcpy(plugin_file_name, plugin);
	if(isupper(plugin_file_name[0])){
		plugin_file_name[0] = tolower(plugin_file_name[0]);
	}
	char* plugin_class_name = (char*)malloc(strlen(plugin)+1);
	strcpy(plugin_class_name, plugin);
	if(!isupper(plugin_class_name[0])){
		plugin_class_name[0] = toupper(plugin_class_name[0]);
	}
	char* whole = (char*)malloc(strlen(from)+strlen(imp)+strlen(ln)+2*strlen(plugin)+1);
	strcpy(whole, from);
	strcat(whole, plugin_file_name);
	strcat(whole, imp);
	strcat(whole, plugin_class_name);
	strcat(whole, ln);
	printf("After everything we got %s\n",whole);
	PyRun_SimpleString(whole);
	free(whole);
	free(plugin_file_name);
	PyObject* main_module = PyImport_AddModule("__main__");
	PyObject* global_dict = PyModule_GetDict(main_module);
	PyObject* local_dict = PyDict_New();
	char* parens = "()\n";
	char* make_obj = (char*)malloc(strlen(plugin_class_name)+strlen(parens)+1);
	strcpy(make_obj, plugin_class_name);
	strcat(make_obj, parens);

	PyObject* plugin = PyRun_String(make_obj,Py_eval_string, global_dict, local_dict);
	free(make_obj);
	free(plugin_class_name);
	Py_DECREF(local_dict);
	obj_list_add(container, plugin);
	*/
}
thread_container make_callback_dict(obj_list plugin_list){
	PyObject* dict = PyDict_New();
	int index = 0;

 
	while(index++ < obj_list_len(plugin_list)){
		PyObject* cur = obj_list_get(plugin_list, index);
		PyObject* cur_plugin_name, cur_plugin_write, getName, plugin_name;
		//Might need to add self reference to args, dunno
		PyObject* tuple = PyTuple_New((Py_ssize_t)0);
		getName = PyObject_GetAttr(cur, "getName");
		plugin_name = PyObject_Call(getName, tuple, NULL);
		Py_DECREF(tuple);
		Py_DECREF(getName);
		cur_plugin_write = PyObject_GetAttr(cur, "addMessage");
		PyDict_SetItem(dict, plugin_name, cur_plugin_write);
		Py_DECREF(plugin_name);
		Py_DECREF(cur_plugin_write);
	}
	return dict;
}
void give_callback_registration_oppertunity(PyObject* plugin, PyObject* call_back_dict){
	PyObject* setup;
	setup = PyObject_GetAttr(plugin, "setup");
	PyObject* arg_tuple = PyTuble_New((Py_ssize_t)2);
	PyTuple_SetItem(arg_tuple, 0, call_back_dict);
	//TODO setup runtimeVars, what are these even?
	PyTuple_SetItem(arg_tuple, 1, Py_None);
	PyObject_Call(setup, arg_tuple, NULL);
	Py_DECREF(arg_tuple);
	Py_DECREF(setup);	
}
void* run_plugin(void* args){
	PyObject* runMethod = (PyObject*)args;
	PyObject* mt_tuple = PyTuple_New(0);
	PyObject_Call(runMethod, mt_tuple, NULL);
	Py_DECREF(mt_tuple);
	Py_DECREF(runMethod);
	pthread_exit(0);
}
//Returns an array of all the threads that are running....
thread_container* init_dds_python(Dict* config){
	Py_Initialize();
	if(dict_has_key(config, "plugins")){
		Dict* val = dict_get_val(config, "plugins");

		
		char* plugins_list, cur_plugin;
		plugins_list_str = val->value;
		obj_list plugin_list = new_object_list();

		cur_plugin = strtok(plugins_list, ",");
		while(cur_plugin){
			import_plugin(cur_plugin);
			init_plugin(cur_plugin, plugins_list);
			cur_plugin = strtok(NULL, ",");
		}
		int index = 0;
		int len = obj_list_len(plugin_list);
		PyObject* cb_dict = make_callback_dict(plugin_list);
		for(;index < len;index++){
			give_callback_registration_oppertunity(obj_list_get(plugin_list, index), cb_dict);
		}
		thread_container* result = make_thread_container();
		PyObject* mt_tuple = PyTuple_New(0);
		for(index = 0; index < len;index++){
			PyObject* cur = obj_list_get(plugin_list, index);
			PyObject* needsThread = PyObject_GetAttr(cur, "needsThread");
			
			PyObject* doesNeed = PyObject_Call(needsThread, mt_tuple, NULL);
			if(PyObject_IsTrue(doesNeed)){
				PyObject* getName = PyObject_GetAttr(cur, "getName");
				PyObject* nameStr = PyObject_Call(getName, mt_tuple, NULL);
				Py_DECREF(getName);
				plugin_thread* tmp_thread = make_plugin_thread(PyString_AS_STRING(nameStr));
				Py_DECREF(nameStr);
				PyObject* runMethod = PyObject_GetAttr(cur, "run");
				pthread_create(&tmp_thread->thread, NULL, run_plugin, (void*)runMethod);
				thread_container_add(tmp_thread);
			}else{
				//TODO figure out what we want to happen here xD
			}
			Py_DECREF(doesNeed);
			Py_DECREF(needsThread);
		}
		Py_DECREF(mt_tuple);
		Py_DECREF(cb_dict);
		del_obj_list(plugin_list);
		return result;
	}
}
