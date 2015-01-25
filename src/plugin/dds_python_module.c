#include <stdio.h>
#include <time.h>
#include <Python.h>
#include <frameobject.h>
#include "../dds_globals.h"
#include "../dict.h"
#include "../parseJSON.h"
#include "dds_python.h"
#include "../dds_io.h"

// v0.1 (Test): Converts to one of our Dict objects & dumps
static PyObject* py_send_message_meth(PyObject *self, PyObject* args){
    if(!args){printf("Error parsing arguments. Returning NULL.\n");return NULL;}
    PyObject *pyDest;
    PyObject *pyPluginDest;
    PyObject *msg_dict;
    Py_ssize_t argc = PyTuple_Size(args);
    if(argc == 2){
        pyDest = NULL;
        pyPluginDest = PyTuple_GetItem(args,0);
        msg_dict = PyTuple_GetItem(args,1);
    }
    else if(argc == 3){
        if(LOCAL_NAME){pyDest = (LOCAL_NAME == PyString_AsString(PyObject_Str(PyTuple_GetItem(args,0)))) ? NULL : PyTuple_GetItem(args,0);}
        else{pyDest = PyTuple_GetItem(args,0);}
        pyPluginDest = PyTuple_GetItem(args,1);
        msg_dict = PyTuple_GetItem(args,2);
    }
    else{printf("Error: dds.send() requires either two or three arguments. Returning NULL.\n"); return NULL;}

    if(pyDest && (!PyString_Check(pyDest))){printf("Error: dds.send() expects its Pi destination to be a string. Returning NULL.\n"); return NULL;}
    if(!PyString_Check(pyPluginDest)){printf("Error: dds.send() expects its Plugin destination to be a string. Returning NULL.\n"); return NULL;}
    if(!PyDict_Check(msg_dict)){printf("Error: dds.send() expects its last argument to be a dictionary. Returning NULL.\n"); return NULL;}
    PyFrameObject *frame = PyEval_GetFrame();
    char* class = PyString_AsString(PyObject_CallMethod(frame->f_localsplus[0],"getName","()"));
    
    time_t curtime;
    time(&curtime);
    struct tm *datetime = malloc(sizeof(struct tm));
    memcpy(datetime,gmtime(&curtime),sizeof(struct tm));

    Dict *dct = parse_py_dict(msg_dict);
    //dump_dict(dct);
    //printf("Thanks for calling me, %s!\n",class);
    Dict *msg = make_dict();
    char *msgcontent = dict_to_raw_json(dct);
    dict_put(msg,"content",DYN_STR(msgcontent));
    char dt[30];
    strftime(dt,30,"%Y-%m2-%d2T%H2-%M2-%S2+00:00",datetime);
    //dict_put(msg,"datetime",dt);
    dict_put(msg,"src",DYN_STR(class));
    dict_put(msg,"pluginDest",DYN_STR(PyString_AsString(pyPluginDest)));
    dict_put(msg,"dest",DYN_STR(PyString_AsString(pyDest)));

    char *to_send = dict_to_raw_json(msg);
    write_to_pipe(to_send);
    // dct freed by dict_to_raw_json...setting to null to be safe
    dct = NULL;

    Py_RETURN_NONE;
}

static PyMethodDef DDSMethods[] = {
    {"send",py_send_message_meth,METH_VARARGS,"Send a message to another plugin or Raspberry Pi"},
    {NULL,NULL,0,NULL}
};

PyMODINIT_FUNC initdds(void){
    (void) Py_InitModule("dds",DDSMethods);
}
