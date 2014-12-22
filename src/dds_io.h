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
int write_s(dds_sock sock, int size, char* str);
int write_int(dds_sock sock, int val);

void err_quit(const char* str);
#endif

