
#include "dict.h"
#include "readcfg.h"
#include "ddsmsgqueue.h"

#define KEY_PATH "/home/pi/c-dds/src/main.c"
int main(void){
	int qid = get_q_id(KEY_PATH, 'd');
	Dict* d = readConfig("../Configs/PIE.conf");

	






	destroy_q(qid);
	//Deallocates the dictionary
	delete_dict_and_contents(d);
}


