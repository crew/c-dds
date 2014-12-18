#include <stddef.h>
#include <sys/msg.h>
#include <stdio.h>
#include <sys/ipc.h>
int get_q_id(const char* path, char type){	
	key_t k = ftok(path, type);
	if(k == -1){
		perror("ftok");
		return -1;
	}
	//TODO error checking xD
	int qid = msgget(k, 0666 | IPC_CREAT);
	if(qid == -1){
		perror("msgget");
		return -1;
	}
	return qid;
}

int destroy_q(int qid){
	int status = msgctl(qid, IPC_RMID, NULL);
	if(status == -1){
		perror("msgctl");
		return 0;
	}else{
		return 1;
	}
}
