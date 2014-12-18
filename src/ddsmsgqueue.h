
#include <sys/msg.h>
#ifndef DDS_MESSAGE_HEAD
#define DDS_MESSAGE_HEAD

typedef struct _dds_msgbuf{
	long mtype;
	void* payload;
}dds_msgbuf;

int get_q_id(const char* path, char type);
int destroy_q(int qid);


#endif
