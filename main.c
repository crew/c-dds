#include <stdio.h>
#include <stdlib.h>
#include "dict.h"
void get_file_size(FILE* file, int* num);
int main(void){
	Dict* d = make_dict();
	FILE* config;
	int file_size;
	config  = fopen("Configs/PIE.conf","r");
	get_file_size(config, &file_size);
	printf("I got size : %d\n",file_size);
	dict_put(d, "size", &file_size);
	printf("Dict has size :%d\n", *(int*)dict_get_val(d, "size"));
}
void get_file_size(FILE* file, int* num){
	int file_pos = ftell(file);
	fseek(file, 0, SEEK_END);
	*num = ftell(file);
	fseek(file, file_pos - *num, SEEK_CUR);
}

