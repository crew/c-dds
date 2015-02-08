#ifndef DICT_HEAD
#define DICT_HEAD
#include "dds_globals.h"


typedef struct _dict{
	char* key;
	void* value;
	VAL_TYPE type;
	struct _dict* next;
}Dict;

Dict* make_dict(void);
Dict* simple_get_val(Dict* d, char* key);
int dict_size(Dict*);
void delete_dict(Dict*);
void delete_dict_and_contents(Dict*);
int dict_has_key(Dict* dict, char* key);
void* DICT_GET_VAL(Dict* dict,  ...);
void* DICT_PUT(Dict* dict, char* key, void* val, VAL_TYPE vtype);
VAL_TYPE DICT_GET_TYPE(Dict* dict, ...);
void* DICT_OVERRIDE_TYPE(Dict* dict, VAL_TYPE new_type, ...);
int dict_remove_entry(Dict* dict, char* key); 
int dict_detatch_entry(Dict* d, char* key);
void dump_dict(Dict*);
#define dict_get_val(...) DICT_GET_VAL(__VA_ARGS__, NULL)
#define dict_get_type(...) DICT_GET_TYPE(__VA_ARGS__, NULL)
#define dict_override_type(...) DICT_OVERRIDE_TYPE(__VA_ARGS__, NULL)
#ifdef __GNUC__
#ifndef __clang__
#if __GNUC__ > 4 || \
              (__GNUC__ == 4 && (__GNUC_MINOR__ >= 9 ) )
#define HASGENERIC
#else
#define NOGENERIC
#endif
#endif
#endif

#ifdef __clang__
#if __has_feature(c_generic_selections) || __has_extension(c_generic_selections)
#define HASGENERIC
#else
#define NOGENERIC
#endif
#endif
#ifdef NOGENERIC
#include <stdio.h>
#ifdef __GNUC__
#ifndef __clang__
#pragma message "WARNING: This version of GCC does not support _Generic(). As such, automatic dictionary typing is disabled and the program may not function correctly.\n\n\tPlease upgrade to GCC version 4.9.\n\n"
#endif
#endif
#ifdef __clang__
#pragma message "WARNING: This version of Clang does not support _Generic(). As such, automatic dictionary typing is disabled and the program may not function correctly.\n\n\tPlease upgrade to Clang version 3.1.\n\n"
#endif
#define dict_put(dct, key, val) DICT_PUT(dct,key,val,T_POINT_VOID)
#endif
#ifdef HASGENERIC
#define TO_TYPE(a) (VAL_TYPE) _Generic((a), \
		int: T_INT, double: T_DOUBLE, char: T_CHAR, \
		int*: T_POINT_INT, char*: T_POINT_CHAR, double*: T_POINT_DOUBLE, struct _dict*: T_DICT, default: T_POINT_VOID)

#define dict_put(DICT_PUT_dct,DICT_PUT_key,DICT_PUT_val) ({__typeof__(DICT_PUT_val) DICT_PUT_PARSED_val = DICT_PUT_val; DICT_PUT(DICT_PUT_dct,DICT_PUT_key,DICT_PUT_PARSED_val,TO_TYPE(DICT_PUT_PARSED_val));})
#endif

#endif
