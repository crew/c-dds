#include "dict.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*
Returns malloc'd memory containing the line from the file 
or NULL if no more can be read
*/
int add_split_to_dict(Dict*,FILE*);

char* next_line(FILE* f){
	char str[128];
	char* p = fgets(str, sizeof(str), f);
	if(p == NULL){
		return NULL;
	}
	if(p[0] == '\n'){
		char* newStr = (char*)malloc(2);
		newStr[0] = '\n';
		newStr[1] = '\0';
		return newStr;
	}
	int len = strlen(p);
	if(p[len - 1] == '\n'){
		p[--len] = '\0';
	}
	//Need one extra space for \0
	char* newStr = (char*)malloc(len+1);
	strcpy(newStr, p);
	return newStr;
	
}
void get_file_size(FILE* file, int* num){
	int file_pos = ftell(file);
	fseek(file, 0, SEEK_END);
	*num = ftell(file);
	fseek(file, file_pos - *num, SEEK_CUR);
}
Dict* readConfig(const char* path){
	FILE* f = fopen(path, "r");
	Dict* d = make_dict();
	while(add_split_to_dict(d, f)){}
	fclose(f);
	return d;
}
int add_split_to_dict(Dict* d, FILE* f){
	char* line = next_line(f);
	if(line == NULL){
		return 0;
	}
	if(line[0] == '\n'){
		free(line);
		return 1;
	}
	int wholeLineLen = strlen(line);
	strtok(line, "=");
	int keyLen = strlen(line)+1;
	int valLen = strlen(line+keyLen)+1;
	char* key = (char*)malloc(keyLen);
	char* val = (char*)malloc(valLen);
	strcpy(key, line);
	strcpy(val, line+keyLen);
	free(line);
	dict_put(d, key, val);
	return !feof(f);
}


