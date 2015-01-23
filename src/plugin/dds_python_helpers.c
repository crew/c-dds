#include <stdio.h>
#include <Python.h>
#include "../dict.h"
#include "../dds_globals.h"
#include "dds_python.h"

static Dict *parse_py_list(PyObject *lst){
    Dict *ret = make_dict();
    char key[10];
    int i = 0;
    int sz = PyList_Size(lst);
    PyObject *value;
    for(;i < sz; i++){
        value = PyList_GET_ITEM(lst,i);
        sprintf(key,"%d",i);
        if(PyDict_Check(value)){
            dict_put(ret,key,parse_py_dict(value));
        }
        else if(PyList_Check(value)){
            dict_put(ret,key,parse_py_list(value));
            dict_override_type(ret,T_ARR,key);
        }
        else if(PyInt_Check(value)){
            int ival = (int)PyInt_AS_LONG(value);
	    dict_put(ret,key,DYN_NON_POINT(ival));
        }
        else if(PyFloat_Check(value)){
            double dval = PyFloat_AsDouble(value);
            dict_put(ret,key,DYN_NON_POINT(dval));
        }
        else if(PyString_Check(value)){
            char *strval = PyString_AsString(value);
            dict_put(ret,key,DYN_STR(strval));
        }
        else{
            printf("WARNING: Unsupported PyObject given. Casting to string representation.");
            char *reprval = PyString_AsString(PyObject_Str(value));
            dict_put(ret,key,DYN_STR(reprval));
        }
    }
    return ret;
}

Dict *parse_py_dict(PyObject *dct){
    Dict *ret = make_dict();
    char *key;
    PyObject *raw_key, *value;
    Py_ssize_t idx = 0;
    while(PyDict_Next(dct,&idx,&raw_key,&value)){
        if(!PyString_Check(raw_key)){ printf("WARNING: Invalid Python dictionary key received. Casting to string representation."); key = PyString_AsString(PyObject_Str(value)); }
        else{key = PyString_AsString(raw_key);}

        if(PyDict_Check(value)){
            dict_put(ret,key,parse_py_dict(value));
        }
        else if(PyList_Check(value)){
            dict_put(ret,key,parse_py_list(value));
            dict_override_type(ret,T_ARR,key);
        }
        else if(PyInt_Check(value)){
            int ival = (int)PyInt_AS_LONG(value);
	    printf("ival: %d\n",ival);
	    dict_put(ret,key,DYN_NON_POINT(ival));
        }
        else if(PyFloat_Check(value)){
            double dval = PyFloat_AsDouble(value);
            dict_put(ret,key,DYN_NON_POINT(dval));
        }
        else if(PyString_Check(value)){
            char *strval = PyString_AsString(value);
            dict_put(ret,key,DYN_STR(strval));
        }
        else{
            printf("WARNING: Unsupported PyObject given. Casting to string representation.");
            char *reprval = PyString_AsString(PyObject_Str(value));
            dict_put(ret,key,DYN_STR(reprval));
        }
    }
    return ret;
}

