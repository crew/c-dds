#ifndef DDS_IO
#define DDS_IO
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define TRY_COUNT 5
typedef int dds_sock;

int net_order(int);
int host_order(int);

dds_sock open_connection(char* addr, char* port);
int close_connection(dds_sock);
//Writes the given string to the socket, gives a non-blocking flag, this call will not block
int write_sb(dds_sock sock, const char* str);
//Writes the string to the socket, allows blocking
int write_snb(dds_sock sock, const char* str);

int write_int(dds_sock sock, int val);

void err_quit(const char* str);
#endif

