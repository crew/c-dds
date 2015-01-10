#include "dds_piobj_list.h"


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
