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
	free(plugin_class_name);
	//Py_eval_string
	//TODO make an instance
	//PyRun_string
}
void give_callback_registration_oppertunity(PyObject* plugin, obj_list all_plugins){
	//TODO hand the given plugin a list of the other plugins and allow it to snag a callback from the ones it needs
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
			init_plugin(cur_plugin, plugins_list);
			cur_plugin = strtok(NULL, ",");
		}
		int index = 0;
		for(;index < obj_list_len(plugin_list);index++){
			give_callback_registration_oppertunity(obj_list_get(plugin_list, index), plugin_list);
		}
		del_obj_list(plugin_list);
	}
}
