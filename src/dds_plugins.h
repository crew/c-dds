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


void init_dds_python(Dict* config);

#endif
