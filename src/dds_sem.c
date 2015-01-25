#include <sys/shm.h>
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

// Python Module Shared Memory Accessors
// TODO: Abstract a little to clean up
#define DDS_PYMOD_SEM "/dds_py_mod_sem"
#define DDS_PYMOD_SHMEM "/dds_py_mod_shmem"

inline dds_sem get_python_module_sem(void){
	dds_sem pym_sem = dds_open_sem(DDS_PYMOD_SEM,1);
	if(!try_dds_sem(pym_sem)){ return NULL; }
	return pym_sem;
}

void *get_python_module_info_ptr(int extra_flags){
	key_t shmemkey = ftok(DDS_PYMOD_SHMEM, 's');
	if(shmemkey == -1){
		perror("ftok");
		return NULL;
	}
	int shmid = shmget(shmemkey, 1024, 0666 | extra_flags);
	if(shmid == -1){
		perror("shmget");
		return NULL;
	}
	void *ret = shmat(shmid,NULL,0);
	if(ret == (void*)-1){
		perror("shmat");
		return NULL;
	}
	return ret;
}

int python_module_info_cleanup(void *addr, dds_sem cur_sem){
	int dt = shmdt(addr);
	if(dt){
		perror("shmdt");
		return 0;
	}
	return release_dds_sem(cur_sem);
}

int set_python_module_info(dds_pymod_info info){
	dds_sem pym_sem = get_python_module_sem();
	if(!pym_sem){ return 0; }
	dds_pymod_info shmptr = get_python_module_info_ptr(IPC_CREAT);
	memcpy(shmptr,info,sizeof(*info));
	return python_module_info_cleanup(shmptr,pym_sem);
}
dds_pymod_info get_python_module_info(void){
	dds_pymod_info ret = malloc(sizeof(struct _dds_pymod_info));
	if(!ret){
		perror("malloc");
		return NULL;
	}
	dds_sem pym_sem = get_python_module_sem();
	if(!pym_sem){ return NULL; }
	dds_pymod_info shmptr = get_python_module_info_ptr(0);
	memcpy(ret,shmptr,sizeof(*shmptr));
	if(!python_module_info_cleanup(shmptr,pym_sem)){
		return NULL;
	}
	return ret;
}
