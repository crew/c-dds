#ifndef DDS_PLUGINS_H
#define DDS_PLUGINS_H

#include <Python.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctypes.h>
#include "dds-globals.h"
#include "dds_piobj_list.h"
#include "dict.h"

#define PLUGINS_FOLDER "Plugins."

typedef struct _plugin_thread{
	char[64] name;
	pthread_t thread;
}plugin_thread;
typedef struct _thread_container{
	int size;
	plugin_thread* thread_arr;
}thread_container;
//Returns an array of plugin threads
 thread_container init_dds_python(Dict* config);

#endif
