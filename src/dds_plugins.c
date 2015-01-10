#include "dds_plugins.h"
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
PyObject* make_callback_dict(obj_list plugin_list){
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
void init_dds_python(Dict* config){
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
		PyObject* cb_dict = make_callback_dict(plugin_list);
		for(;index < obj_list_len(plugin_list);index++){
			give_callback_registration_oppertunity(obj_list_get(plugin_list, index), cb_dict);
		}
		Py_DECREF(cb_dict);
		del_obj_list(plugin_list);
	}
}
