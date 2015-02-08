#include "dds_sem.h" 
dds_sem dds_open_sem(const char* name, int value){
	if(*name != '/'){
		printf("WARNING: %s does not follow semaphore naming conventions...\n", name);
	}
	dds_sem dsem = (dds_sem)malloc(sizeof(_dds_sem));
	if(strlen(name) > DDS_SEM_NAME_MAX){
		printf("WARNING: dds_sem names cannot be longer then %d chars, truncating %s at %d chars...\n", DDS_SEM_NAME_MAX, name, DDS_SEM_NAME_MAX);
	}
	strncpy(dsem->name, name, DDS_SEM_NAME_MAX);
	dsem->sem = sem_open(name, O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, value);
	if(dsem->sem == SEM_FAILED){
		perror("sem_open");
		return NULL;

	}
	return dsem;
}
int close_dds_sem(dds_sem s){
	int ret = 1;
	if(sem_close(s->sem) == -1){
		perror("sem_close");
		ret = 0;
	}
	free(s);
	return ret;
}
int final_close_sem(dds_sem sem){
	int ret = 1;
	if(sem_unlink(sem->name) == -1){
		perror("sem_unlink");
		ret = 0;
	}
	free(sem);
	return ret;
}
int try_dds_sem(dds_sem sem){
	if(sem_wait(sem->sem) == -1){
		perror("sem_trywait");
		return 0;

	}
	return -1;

}
int release_dds_sem(dds_sem sem){
	if(sem_post(sem->sem) == -1){
		perror("sem_post");
		return 0;

	}
	return 1;
}
