#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dict.h"
void get_file_size(FILE* file, int* num);
//NOTE: must free return value after use
char* next_line(FILE* file);
int add_split_to_dict(Dict* d, FILE* f); 
int main(void){
	
	Dict* d = make_dict();
	FILE* config;
	int file_size;
	config  = fopen("../Configs/PIE.conf","r");
	while(add_split_to_dict(d, config)){}
	printf("Val for key 12345 is %s\n", (char*)dict_get_val(d, "12345"));
	printf("Val for key two is %s\n",(char*)dict_get_val(d, "two"));
	

}
int add_split_to_dict(Dict* d, FILE* f){
	char* line = next_line(f);
	if(line == NULL){
		return 0;
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
	dict_put(d, key, (void*)val);
	return !feof(f);
}

/*
Returns malloc'd memory containing the line from the file 
or NULL if no more can be read
*/
char* next_line(FILE* f){
	char str[128];
	char* p = fgets(str, sizeof(str), f);
	if(p == NULL){
		return NULL;
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

