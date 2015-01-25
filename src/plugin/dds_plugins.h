#ifndef DDS_PLUGINS_H
#define DDS_PLUGINS_H

#include <Python.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "../dds_globals.h"
#include "../dict.h"
#include "dds_piobj_list.h"
#include "dds_python.h"

#define PLUGINS_FOLDER "Plugins."
#define PLUGINS_PATH "Plugins."
struct listener_args { pthread_mutex_t* mtx; pthread_cond_t* cond; char* name; PyObject *add_msg_method; };
struct run_args { struct listener_args largs; PyObject *others; pthread_t* listener_t;};
typedef struct _plugin_thread{
	char name[64];
	pthread_t thread;
	pthread_t listener_thread;
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
	struct run_args *rargs;
}plugin_thread;
typedef struct _thread_container{
	int size;
	plugin_thread** thread_arr;
}thread_container;
thread_container* make_thread_container(void);
plugin_thread* make_plugin_thread(char* name);
void thread_container_add(thread_container* container, plugin_thread* thread);
void delete_plugin_thread(plugin_thread* plug);
void delete_thread_container(thread_container* c);
void send_plugin_message(char*,Dict*);
//Returns an array of plugin threads
thread_container* init_dds_python(Dict* config);

#endif
