#ifndef DDS_SEM_H
#define DDS_SEM_H
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#define DDS_SEM_NAME_MAX 64
typedef struct _dds_sem{
	sem_t* sem;
	char name[DDS_SEM_NAME_MAX];
}_dds_sem;
typedef _dds_sem* dds_sem;

typedef struct _dds_pymod_info {
	int sock_fd;
	char local_name[64];
}* dds_pymod_info;

dds_sem dds_open_sem(const char* name, int value);
int close_dds_sem(dds_sem s);
int final_close_sem(dds_sem s);

int try_dds_sem(dds_sem sem);
int release_dds_sem(dds_sem sem);

int set_python_module_info(dds_pymod_info);
dds_pymod_info get_python_module_info(void);
#endif
