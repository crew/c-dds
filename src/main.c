#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dict.h"
#include "readcfg.h" 
int main(void){
	Dict* d = readConfig("../Configs/PIE.conf");
	printf("Val for key 12345 is %s\n", (char*)dict_get_val(d, "12345"));
	printf("Val for key two is %s\n",(char*)dict_get_val(d, "two"));
	delete_dict_and_contents(d);
}


