/*
 * dds-globals.h
 *
 *  Created on: Dec 29, 2014
 *      Author: belph, c0d3d
 */

#ifndef SRC_DDS_GLOBALS_H_
#define SRC_DDS_GLOBALS_H_
#ifdef ___TEST_SUITES___
#include "../tests/CuTest.h"
#endif

#include <string.h>
#define DYN_STR(c) (strcpy((char*)malloc(strlen(c)+1),c))
#define DYN_NON_POINT(a) (memcpy((__typeof__(a)*)malloc(sizeof(a)),&a,sizeof(a)))

extern int in_heap(void *address);

#endif /* SRC_DDS_GLOBALS_H_ */
