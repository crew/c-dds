
#include "dds_plugins.h"
Dict* listener_flags;
Dict* msg_queue;

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
	pthread_mutex_init(&(tmp->mutex), NULL);
	pthread_cond_init(&(tmp->cond),NULL);
	printf("Adding Listener Flag for %s to Dictionary.\n",name);
	dict_put(listener_flags,name,(void*)(&(tmp->cond)));
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
	/*if(isupper(myplg[0])){
		myplg[0] = tolower(myplg[0]);
	}*/
	strcpy(path, PLUGINS_PATH);
	strcat(path, myplg);
	PyObject* plug_module = PyImport_ImportModule(path);
	if(!plug_module){
		printf("Couldn't import the module plugin module...\n");
		PyErr_Print();
	}
	/*if(islower(myplg[0])){
		myplg[0] = toupper(myplg[0]);

	}*/
	PyObject* clazz = PyObject_GetAttrString(plug_module, myplg);
	if(!clazz){
		printf("Couldn't get the plugin class...\n");
		PyErr_Print();
	}
	//TODO check for plugin subclassing
	Py_DECREF(plug_module);
	free(myplg);
	free(path);
	if(!PyClass_Check(clazz)){
		fprintf(stderr, "Got something that wasn't a class object...\n");
	}

	PyObject* plugin_instance = PyInstance_New(clazz, NULL, NULL);
	if(!plugin_instance){
		printf("Couldn't make plugin instance...\n");
		PyErr_Print();
	}
	Py_DECREF(clazz);
	obj_list_add(container, plugin_instance);
}
PyObject* make_callback_dict(obj_list plugin_list){
	PyObject* dict = PyDict_New();
	int index = 0;

 
	while(index < obj_list_len(plugin_list)){
		PyObject* cur = obj_list_get(plugin_list, index);
		//Might need to add self reference to args, dunno
		PyObject* tuple = PyTuple_New((Py_ssize_t)0);
		if(!tuple){
			printf("Couldn't make tuple ?\n");
			PyErr_Print();
		}
		PyObject* getName = PyObject_GetAttrString(cur, "getName");
		if(!getName){
			printf("Couldn't get the getName method...\n");
			PyErr_Print();
		}
		PyObject* plugin_name = PyObject_Call(getName, tuple, NULL);
		if(!plugin_name){
			printf("Couldn't call the getName method...\n");
			PyErr_Print();
		}
		Py_DECREF(tuple);
		Py_DECREF(getName);
		PyObject* cur_plugin_write = PyObject_GetAttrString(cur, "addMessage");
		if(!cur_plugin_write){
			printf("Couldn't get addMessage method...\n");
			PyErr_Print();
		}
		if(!PyMethod_Check(cur_plugin_write) && !PyFunction_Check(cur_plugin_write)){
			printf("Not function or method...\n");
		}
		printf("Does it have the __call__ attribute %d\n", PyObject_HasAttrString(cur_plugin_write, "__call__"));
		PyDict_SetItem(dict, plugin_name, cur_plugin_write);
		//Py_DECREF(plugin_name);
		//Py_DECREF(cur_plugin_write);
		index++;
	}
	return dict;
}
void give_callback_registration_oppertunity(PyObject* plugin, PyObject* call_back_dict){
	PyObject* setup;
	setup = PyObject_GetAttrString(plugin, "setup");
	if(!setup){
		printf("Couldn't get setup method...\n");
		PyErr_Print();
	}
	PyObject* arg_tuple = PyTuple_New((Py_ssize_t)1);
	if(!arg_tuple){
		printf("Couldn't make the argument tuple...\n");
		PyErr_Print();
	}
	PyTuple_SetItem(arg_tuple, 0, Py_None);
	//PyTuple_SetItem(arg_tuple, 0, call_back_dict);
	//TODO setup runtimeVars, what are these even?
	//PyTuple_SetItem(arg_tuple, 1, Py_None);
	PyObject* ret = PyObject_Call(setup, arg_tuple, NULL);
	if(!ret){
		printf("Couldn't call setup...\n");
		PyErr_Print();
	}else{
		Py_DECREF(ret);//Pretty sure we need to decrement the ref count for None
	}
	Py_DECREF(arg_tuple);
	Py_DECREF(setup);	
}

void send_plugin_message(char* name, Dict* msg){
	if(dict_has_key(listener_flags,name)){
		while(dict_has_key(msg_queue,name)){}
		printf("Message queued for plugin %s\n", name);
		dict_put(msg_queue,DYN_STR(name),msg);
		pthread_cond_signal((pthread_cond_t*)dict_get_val(listener_flags,name));
	}
}

void* message_listener(void* rawargs){
	struct listener_args args = *(struct listener_args*)rawargs;
	//PyEval_AcquireLock();
	// Main interpreter state
	//PyInterpreterState *mis = args.mts->interp;
	//PyThreadState *this_thread_state = PyThreadState_New(mis);
	//PyEval_ReleaseLock();
	while(1) {
		pthread_cond_wait(args.cond, args.mtx);
		//PyEval_AcquireLock();
		//PyThreadState_Swap(this_thread_state);
		//do Py_BEGIN_ALLOW_THREADS
		PyObject *msg_tuple = PyTuple_New(1);
		PyTuple_SetItem(msg_tuple,0,(parse_dict_to_pydict(dict_get_val(msg_queue,args.name))));
		printf("Sending message to %s: ", args.name); 
		PyObject_Print(PyTuple_GetItem(msg_tuple,0), stdout, 0);
		printf("\n");
		//PyGILState_STATE gil = PyGILState_Ensure();
		printf("Result:");
		PyObject_Print(PyObject_Call(args.add_msg_method, msg_tuple, NULL), stdout, 0);
		printf("\n");
		//PyGILState_Release(gil);
		//PyThreadState_Swap(NULL);
		//PyEval_ReleaseLock();
		Py_DECREF(msg_tuple);
		//Py_END_ALLOW_THREADS while(0);
		dict_remove_entry(msg_queue,args.name);
	}
}


void* run_plugin(void* rawargs){
	struct run_args args = *(struct run_args*)rawargs;
	//Py_Initialize();
	//PyGILState_STATE gil = PyGILState_Ensure();
	if(args.listener_t){
		struct listener_args to_pass = args.largs;
		pthread_create(args.listener_t,NULL, message_listener, &to_pass);
	}
	PyObject* runMethod = args.others;
	PyObject* mt_tuple = PyTuple_New(0);
	PyObject_Call(runMethod, mt_tuple, NULL);
	Py_DECREF(mt_tuple);
	Py_DECREF(runMethod);
	//PyGILState_Release(gil);
	pthread_exit(0);
}
//Returns an array of all the threads that are running....
thread_container* init_dds_python(Dict* config){
	listener_flags = make_dict();
	msg_queue = make_dict();
	setenv("PYTHONPATH", "..", 0);
	Py_Initialize();
	PyEval_InitThreads();
	PyThreadState *main_thread_state = PyThreadState_Get();
	//PyGILState_STATE gil = PyGILState_Ensure();
	if(dict_has_key(config, "plugins")){
		char* val = dict_get_val(config, "plugins");

		
		char* cur_plugin;
		char* plugins_list_str = val;
		obj_list plugin_list = new_object_list();

		cur_plugin = strtok(plugins_list_str, ",");
		while(cur_plugin){
			//import_plugin(cur_plugin);
			init_plugin(cur_plugin, plugin_list);
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
			PyObject* needsThread = PyObject_GetAttrString(cur, "needsThread");
			if(!needsThread){
				printf("Couldn't get the needsThread method...\n");
				PyErr_Print();
			}
			PyObject* doesNeed = PyObject_Call(needsThread, mt_tuple, NULL);
			if(!doesNeed){
				printf("Couldn't call the needsThread method...\n");
				PyErr_Print();
			}
			if(PyObject_IsTrue(doesNeed)){
				PyObject* getName = PyObject_GetAttrString(cur, "getName");
				if(!getName){
					printf("Couldn't get the getName function...\n");
					PyErr_Print();
				}
				PyObject* nameStr = PyObject_Call(getName, mt_tuple, NULL);
				if(!nameStr){
					printf("Couldn't call the get name function...\n");
					PyErr_Print();
				}
				Py_DECREF(getName);
				plugin_thread* tmp_thread = make_plugin_thread(PyString_AS_STRING(nameStr));
				int has_add_msg = PyDict_Contains(cb_dict, nameStr);
				PyObject *add_message;
				if(has_add_msg){add_message = PyDict_GetItem(cb_dict, nameStr); }
				Py_DECREF(nameStr);
				PyObject* runMethod = PyObject_GetAttrString(cur, "run");
				if(!runMethod){
					printf("Couldn't get the run method...\n");
					PyErr_Print();
				}else{
				//	PyGILState_Release(gil);
					if(has_add_msg){
						struct listener_args largs = { &(tmp_thread->mutex), &(tmp_thread->cond), tmp_thread->name, main_thread_state, add_message };
						struct run_args to_pass = { largs, runMethod, &(tmp_thread->listener_thread) };
						tmp_thread->rargs = malloc(sizeof(to_pass));
						memcpy(tmp_thread->rargs,&to_pass,sizeof(to_pass));
						pthread_create(&tmp_thread->thread, NULL, run_plugin, tmp_thread->rargs);
					}
					else{
						struct run_args to_pass = { (struct listener_args){NULL, NULL, NULL, NULL}, runMethod, NULL };
						tmp_thread->rargs = malloc(sizeof(to_pass));
						memcpy(tmp_thread->rargs,&to_pass,sizeof(to_pass));
						pthread_create(&tmp_thread->thread, NULL, run_plugin, tmp_thread->rargs);
					}
				//	gil = PyGILState_Ensure();
				
				}
				thread_container_add(result, tmp_thread);
			}else{
				//TODO figure out what we want to happen here xD
			}
			Py_DECREF(doesNeed);
			Py_DECREF(needsThread);
		}
		Py_DECREF(mt_tuple);
		Py_DECREF(cb_dict);
		del_object_list(plugin_list);
		PyEval_ReleaseLock();
		//PyGILState_Release(gil);
		return result;
	}
}
