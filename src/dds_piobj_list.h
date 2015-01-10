#ifndef DDS_PIOBJ_LIST_H
#define DDS_PIOBJ_LIST_H

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
#endif
