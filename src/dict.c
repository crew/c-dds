#include "dict.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
Dict* make_dict_with(char* key, void* value){
	Dict* d = (Dict*) malloc(sizeof(Dict));
	d->key = key;
	d->value = value;
	d->next = NULL;
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
	return dict_get_val(d,key) != NULL;
}
void* dict_get_val(Dict* d, char* key){
	//Not sure if i can just use d as cur?
	Dict* cur = d->next;
	while(cur != NULL){		
		if(!strcmp(key, cur->key)){
			return cur->value;
		}
		cur = cur->next;
	}
	return NULL;
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