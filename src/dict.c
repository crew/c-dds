#include "dict.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

Dict* make_dict_with(char* key, void* value){
	Dict* d = (Dict*) malloc(sizeof(Dict));
	d->key = key;
	d->value = value;
	d->next = NULL;
	return d;
}
Dict* make_dict(void){
	return make_dict_with(NULL,NULL);
}
int dict_size(Dict* d){
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
	if(freeContents){
		free(index->key);
		free(index->value);
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
	return DICT_GET_VAL(d,key,"\0") != NULL;
}
void* DICT_GET_VAL(Dict* d, char* key, ...){
	//Not sure if i can just use d as cur?
	if (d == NULL){return NULL;}
	Dict* cur = d->next;
	void *ret = NULL;
	va_list arguments;
	char *k;
	while(cur != NULL){		
		if(!strcmp(key, cur->key)){
			ret = cur->value;
			break;
		}
		cur = cur->next;
	}
	va_start(arguments,key);
	while(((k = va_arg(arguments, char *)) != NULL) && (k[0] != '\0')){
		void *temp = DICT_GET_VAL(((Dict *)ret),k);
		ret = (temp == NULL) ? ret : temp;
	}
	va_end(arguments);
	return ret;
}
void* dict_put(Dict* d, char* key, void* val){
	if(d->next == NULL){
		d->next = make_dict_with(key, val);
		return NULL;
	}
	Dict* index = d->next;
	while(index->next != NULL){
		if(!strcmp(key, index->key)){
			void* old = index->value;
			index->value = val;
			return old;
		}
		index = index->next;
	}
	index->next = make_dict_with(key,val);
	return NULL;
}
int dict_remove_entry(Dict* d, char* key){
	Dict* prev = d;
	Dict* index = d->next;
	while(index != NULL){
		if(!strcmp(key, index->key)){
			prev->next = index->next;
			free(index);
			return 1;
		}
		prev = index;
		index = index->next;
	}
	return 0;
}

char *dump_rest(Dict *d){
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
}
