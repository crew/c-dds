#include <Python.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctypes.h>

#include "dict.h"
struct _pyobj_list{
	PyObject* plist;
};
typedef struct _pyobj_list* obj_list;


obj_list new_object_list(void);
void del_object_list(obj_list list);

PyObject* obj_list_get(obj_list list, int index);
PyObject* obj_list_remove(obj_list list, int index);
int obj_list_add(obj_list list, PyObject* obj);
int obj_list_len(obj_list list);
PyObject* obj_list_get(obj_list list, int index){
	Py_ssize_t size = PyList_GET_SIZE(list->plist);
	if(index < size && index >= 0){
		return PyList_GetItem(list->plist, (Py_ssize_t)index); 
	}else{
		return NULL;
	}
}
PyObject* obj_list_remove(obj_list list, int index){
	Py_ssize_t size = PyList_GET_SIZE(list->plist);
	if(index < 0 || index >= size){
		return NULL;
	}
	PyObject* at = PyList_GetItem(list->plist, index);
	PyObject* low_slice = PyList_GetSlice(list->plist, 0, (Py_ssize_t)index);
	PyObject* high_slice = PyList_GetSlice(list->plist, (Py_ssize_t)(index+1), size);
	Py_DECREF(list->plist);
	Py_ssize_t index = 0;
	for(;index < PyList_GET_SIZE(high_slice);index++){
		if(PyList_Append(low_slice, PyList_GetItem(high_slice, index)) == -1){
			//Something bad happened...
			fprintf(stderr, "Something when wrong when removing an item from python list...\n");
		}
	}
	Py_DECREF(high_slice);
	list->plist = low_slice;
	return at;
}
int obj_list_add(obj_list list, PyObject* obj){
	return PyList_Append(list->plist, obj) == 0;
}

obj_list new_object_list(void){
	obj_list l = (obj_list)malloc(sizeof(struct _pyobj_list));
	l->plist = PyList_New((Py_ssize_t) 0);
	return l;
}
void del_object_list(obj_list list){
	Py_DECREF(list->plist);
	free(list);
}
int obj_list_len(obj_list list){
	return (int)PyList_GET_SIZE(list->plist);
}
void init_plugin(char* plugin, obj_list container){
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
