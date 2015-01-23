#include <stdio.h>
#include <Python.h>
#include "dds_globals.h"
#include "dds_python.h"
#include "dict.h"

// v0.1 (Test): Converts to one of our Dict objects & dumps
static PyObject* py_send_message_meth(PyObject *self, PyObject* msg_dict){
    if(!msg_dict){printf("Error parsing arguments. Returning NULL.");return NULL;}
    if(!PyDict_Check(msg_dict)){printf("Error: dds.send() expects a dictionary. Returning NULL."); return NULL;}
    Dict *dct = parse_py_dict(msg_dict);
    dump_dict(dct);
    delete_dict_and_contents(dct);
    Py_RETURN_NONE;
}

static PyMethodDef DDSMethods[] = {
    {"send",py_send_message_meth,METH_O,"Send a message to another plugin or Raspberry Pi"},
    {NULL,NULL,0,NULL}
};

PyMODINIT_FUNC initdds(void){
    (void) Py_InitModule("dds",DDSMethods);
}
