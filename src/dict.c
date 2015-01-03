#include "dict.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
//PASSED MEMORY CHECKS (ASSUMING KEYS ARE ALSO MALLOC'ED)
Dict* make_dict_with(char* key, void* value){
	Dict* d = (Dict*) malloc(sizeof(Dict));
	d->key = key;
	d->value = value;
	if(!key && !value){d->type = T_NULL;} // Empty dictionary
	d->next = NULL;
	return d;
}
Dict* make_dict(void){
	return make_dict_with(NULL,NULL);
}
int dict_size(Dict* d){
	if(!d){
		printf("ERROR: dict_size: NULL Dict* received. Raising SIGTRAP.\n");
		raise(5); // (Just in case you didn't know, SIGTRAP causes a gdb breakpoint)
	}
	if(d->next == NULL){
		return 0;
	}
	Dict* i = d->next;
	int count = 1;
	while(i->next != NULL){
		++count;
		i = i->next;
	}
	return count;
}
void del_last(Dict* d, int freeContents){
	Dict* beforeLast = d;
	Dict* index = d;
	while(index->next != NULL){
		beforeLast = index;
		index = index->next;
	}
    if(index->type == T_POINT_CHAR){
        printf("Removing last element with %s:%s\n", index->key, (char*)index->value);
    }
	if(freeContents){
        if(index->type == T_DICT){
            delete_dict_and_contents((Dict*)index->value);
        }
        else{
        	if(index->type != T_NULL){free(index->value);}
        }
		free(index->key);

		index->key = NULL;
		index->value = NULL;
	}
	free(index);
	beforeLast->next = NULL;
}
void _delete_dict(Dict* d, int freeContents){
	int size = dict_size(d);
	int i = 0;
	for(i; i<size;i++){
		del_last(d, freeContents);
	}
	free(d);
}
void delete_dict(Dict* d){
	_delete_dict(d, 0);
}
void delete_dict_and_contents(Dict* d){
	_delete_dict(d, 1);
}
int dict_has_key(Dict* d, char* key){
	return DICT_GET_VAL(d,key,NULL) != NULL;
}
Dict* DICT_GET_KEYPAIR(Dict* d, va_list arguments){
	if (d == NULL){return NULL;}
	va_list args;
	va_copy(args,arguments);
	Dict *cur = d->next;
	void *ret = NULL;
	char *k;
	_Bool first = 1;
	while((k = va_arg(args, char *)) != NULL){
		if(first){first = 0;}
		else{cur = ((Dict*)((Dict*)ret)->value)->next;}
		if(!cur){printf("WARNING: Key %s not found in dictionary. Returning null.\n", k); return NULL;}
		while(cur != NULL){
			if(!strcmp(k, cur->key)){
				ret = cur;
				break;
			}
			cur = cur->next;
		}
	}
	return (Dict*)ret;
}
void* DICT_GET_VAL(Dict* d, ...){
	va_list args;
	va_start(args,d);
	void *kp = DICT_GET_KEYPAIR(d,args);
	va_end(args);
	return (kp == NULL) ? kp : ((Dict*)kp)->value;
}
void* DICT_PUT(Dict* d, char* key, void* val, VAL_TYPE vtype){
	if(!val){vtype = T_NULL;}
    if(vtype == T_POINT_CHAR){
        printf("Adding %s:%s\n",key, (char*)val);
    }
	if(d->next == NULL){
		d->next = make_dict_with(key, val);
		d->next->type = vtype;
		return NULL;
	}
	Dict* index = d->next;
	while(index->next != NULL){
		if(!strcmp(key, index->key)){
			void* old = index->value;
			index->value = val;
			index->type = vtype;
			return old;
		}
		index = index->next;
	}
	index->next = make_dict_with(key,val);
	index->next->type = vtype;
	return NULL;
}
VAL_TYPE DICT_GET_TYPE(Dict* dct, ...){
	va_list args;
	va_start(args, dct);
	Dict *kp = DICT_GET_KEYPAIR(dct,args);
	va_end(args);
	return kp->type;
}
void* DICT_OVERRIDE_TYPE(Dict* dct, VAL_TYPE new_type, ...){
	va_list args;
	va_start(args,new_type);
	Dict *kp = DICT_GET_KEYPAIR(dct,args);
	va_end(args);
	if(!kp->value){
		printf("WARNING: Overriding type of null pointer at dictionary key %s.\n",kp->key);
	}
	kp->type = new_type;
	return kp;
}

int _dict_remove_entry(Dict *d, char* key, _Bool free_entry){
	Dict* prev = d;
	Dict* index = d->next;
	while(index != NULL){
		if(!strcmp(key, index->key)){
			prev->next = index->next;
            //TODO might need to do some more free-ing here, or we could leave it up to user
			if(free_entry){free(index);}
			return 1;
		}
		prev = index;
		index = index->next;
	}
	return 0;
}

int dict_remove_entry(Dict* d, char* key){
	return _dict_remove_entry(d,key,1);
}

// DOES NOT FREE VALUE; ONLY DETATCHES IT FROM DICTIONARY
// USE WITH EXTREME CAUTION
int dict_detatch_entry(Dict* d, char* key){
	return _dict_remove_entry(d,key,0);
}

// T_INT, T_DOUBLE, T_CHAR, T_POINT_INT, T_POINT_DOUBLE, T_POINT_CHAR, T_POINT_VOID, T_ARR, T_DICT

void dump_dict_atom(Dict *mem, int indents);
void dump_dict_dict(Dict *d, int indents);

void dump_dict_dict(Dict *d, int indents){
	int i = 0;
	char tabs[indents + 1];
	for(;i<indents;i++){
		tabs[i] = '\t';
	}
	tabs[indents] = '\0';
	int len = dict_size(d);
	i = 0;
	Dict *temp = d;
	for(;i<=len;i++){
		printf("%s\"%s\"\t: ",tabs,temp->key);
		dump_dict_atom(temp, indents);
		printf((i < len-1) ? ",\n" : "\n");
		temp = temp->next;
	}
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
void dump_dict_atom(Dict *mem, int indents){
	int i = 0;
	char tabs[indents + 1];
	for(;i<indents;i++){
		tabs[i] = '\t';
	}
	tabs[indents] = '\0';
	if(mem->type == T_INT){
		printf("%d",*(int *)mem->value);
	}
	else if(mem->type == T_DOUBLE){
		printf("%f",*(double *)mem->value);
	}
	else if(mem->type == T_CHAR){
		printf("'%c'",*(char *)mem->value);
	}
	else if(mem->type == T_POINT_INT){
		printf("%d (0x%x)", *(int *)mem->value, (int)mem->value);
	}
	else if(mem->type == T_POINT_CHAR){
		printf("\"%s\" (0x%x)", (char *)mem->value, (int)mem->value);
	}
	else if(mem->type == T_POINT_DOUBLE){
		printf("%f (0x%x)", *(double *)mem->value, (int)mem->value);
	}
	else if(mem->type == T_POINT_VOID){
		printf("<void pointer> (0x%x)",(int)mem->value);
	}
	else if(mem->type == T_NULL){
		printf("<null pointer> (0x0)");
	}
	else if(mem->type == T_ARR){
		printf("Array: [\n");
		dump_dict_dict(((Dict*)mem->value)->next, indents + 1);
		printf("%s\t\t]",tabs);
	}
	else if(mem->type == T_DICT){
		printf("Dict: {\n");
		dump_dict_dict(((Dict*)mem->value)->next, indents + 1);
		printf("%s\t\t}",tabs);
	}
}
#pragma GCC diagnostic pop

void dump_dict(Dict *dct){
	printf("Dict: {\n");
	dump_dict_dict(dct->next, 1);
	printf("}\n");
}

/*char *dump_rest(Dict *d){
	if(d == NULL){
		return "}\n";
	}
	char *ret;
	ret = malloc(80 * sizeof(char));
	sprintf(ret, "\t%s\t:\t%s,\n", d->key, (char *)d->value);
	strcat(ret, dump_rest(d->next));
	return ret;
}

char *dump_dict(Dict *d){
	char *ret, *app;
	app = dump_rest(d);
	ret = malloc((2 * sizeof(char)) + sizeof(app));
	*ret = '{';
	*(ret+1) = '\n';
	*(ret+2) = '\0';
	strcat(ret, dump_rest(d));
	return ret;
}*/
