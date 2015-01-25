#ifndef DDS_PYTHON_H
#define DDS_PYTHON_H
#include "../dict.h"

Dict *parse_py_dict(PyObject *dct);
PyObject *parse_dict_to_pydict(Dict*);
#endif
