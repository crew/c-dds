#ifndef DDS_IO
#define DDS_IO
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>

#define TRY_COUNT 5
#define DDS_END_MSG '\v'
typedef struct _dds_sock{
	int fd;
	int bytes;
	int msgs;
	char* data;
}_dds_sock;


typedef _dds_sock*  dds_sock;

int net_order(int);
int host_order(int);

dds_sock plugin_listener(void);
dds_sock open_connection(char* addr, char* port);
int close_connection(dds_sock);
//Writes the given string to the socket, gives a non-blocking flag, this call will not block
int write_sb(dds_sock sock, const char* str, int size);
//Writes the string to the socket, allows blocking
int write_snb(dds_sock sock, const char* str, int size);

int msg_complete(dds_sock s);

//Gets the number of messages that have been read (or have read in progress read) and not retrieved
int get_msg_count(dds_sock s);

//Does this socket have an in progress read?
int msg_in_progress(dds_sock s);

//Message must be done beaing read, or will return -1
//Acts like a FIFO queue in the case there are multiple messages
//REmoves the given message from the sock's internals so that the next call
//will provide a different message
//buff must be big enough to accomidate the entire message
int get_msg(dds_sock s, char* buff);

//Gets the size (in bytes) of the next msg in this socket (or -1 if it hasn't been completed yet)
int get_nxt_msg_size(dds_sock);
//prrforms a read, allows blocking for entire message
int read_b(dds_sock s, int amt);
//performs a read, does not allow blocking
int read_db(dds_sock s, int amt);

void write_to_pipe(char*);
char *read_from_pipe(void);

void err_quit(const char* str);
#endif

