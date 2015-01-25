#include "dds_io.h"
//PASSED MEMCHECK (BY EYE) ALL POINTERS MUST HAVE BEEN FREED
int dorecv(dds_sock s, char* buf, int amt, int flags){
	ssize_t bytes_read = recv(s->fd, buf, amt, flags);
	if(bytes_read == -1){
		int err = errno;
		if((err == EAGAIN || err == EWOULDBLOCK)){
			return 0;
		}
		perror("recv");
		return 0;

	//	err_quit("Error while recieving message...");
	}
	int newsize = s->bytes + bytes_read;

	char* resized = (char*)malloc(newsize);
	memcpy(resized, s->data, s->bytes);
	memcpy(resized+(s->bytes), buf, bytes_read);
	free(s->data);
	s->data = resized;
	s->bytes = newsize;
	int count = 0;
	int index = 0;
	for(;index < newsize;++index){
		if(resized[index] == '\v'){
			++count;
		}
	}
	s->msgs = count;
	return bytes_read;
}
//prrforms a read, allows blocking for entire message
int read_b(dds_sock s, int amt){
	//TODO Needs to append any read data to data, checks for end msg and marks msgs, also needs to update bytes
	int bytes_read = 0;
	char* buf = (char*)malloc(amt);
	bytes_read = dorecv(s, buf, amt, 0);
	free(buf);
	if(bytes_read == 0){
		printf("No messages and the connection has been terminated by the peer...\n");
	}
	return bytes_read;
}
//performs a read, does not allow blocking
int read_db(dds_sock s, int amt){
	char* buf = (char*)malloc(amt);
	int bytes_read = dorecv(s, buf, amt, MSG_DONTWAIT);
	free(buf);
	if(bytes_read == 0){
	//	printf("No messages availible during non-waiting recv...\n");
	}
	return bytes_read;
}

//returns 1 if the sock has a complete message
int msg_complete(dds_sock s){
	return s->msgs > 0;
}

//Gets the number of messages that have been read (or have read in progress read) and not retrieved
int get_msg_count(dds_sock s){
	return s->msgs;
}

//Does this socket have an in progress read?
int msg_in_progress(dds_sock s){
	if(s->bytes <= 1){
		return 0;
	}
	//NOTE are message terminating convention is to have \v at the end of a message
	return s->data[s->bytes - 2] != DDS_END_MSG;
}
int get_first_end_index(char* c, int len){
	int index = 0;
	while(c[index++] != DDS_END_MSG){
		if(index >= len)
			return -1;
	}
	return --index;
}
int replace_first_end(char* c, int len){
	int f = get_first_end_index(c, len);
	if(f == -1){
		return 0;
	}

	c[f] = '\0';
	return 1;
}
//Message must be done beaing read, or will return -1
//Acts like a FIFO queue in the case there are multiple messages
//REmoves the given message from the sock's internals so that the next call
//will provide a different message
//buff must be big enough to accomidate the entire message
int get_msg(dds_sock s, char* buff){
	if(s->msgs <= 0)
		return 0;
	replace_first_end(s->data, s-> bytes);
	int msg_len = strlen(s->data)+1;
	int rest_len = s->bytes - msg_len;
	strcpy(buff, s->data);
	char* ndata = (char*)malloc(rest_len);
	memcpy(ndata, s->data+msg_len, rest_len);
	free(s->data);
	s->data = ndata;
	s->bytes = rest_len;
	--s->msgs;

	return 1;
}

//Gets the size (in bytes) of the next msg in this socket (or -1 if it hasn't been completed yet)
int get_nxt_msg_size(dds_sock s){
	int l = get_first_end_index(s->data, s->bytes);
	if(l == -1)
		return -1;
	else
		return l+1;
}



void err_quit(const char* str){
	fprintf(stderr, "%s\n%s\n",str, strerror(errno));
	exit(errno);
}
//int read_s(dds_sock s, 
int write_s(dds_sock s, const char* str, int size, int flags){
	int bytes = send(s->fd, str, size, flags);
	char *vtab = "\v";
	send(s->fd, vtab, 1, flags);
	if(bytes == -1){
		perror("send");
		exit(errno);
	}
	return bytes;
}
int write_sb(dds_sock s, const char* str, int size){
	return write_s(s,str, size, 0);
}
int write_snb(dds_sock s, const char* str, int size){
	return write_s(s, str, size, MSG_DONTWAIT);
}

int net_order(int i){
	return htonl(i);
}
int close_connection(dds_sock sock){
	int err = close(sock->fd);
	free(sock->data);
	free(sock);
	if(err == 0)
		return 0;
	perror("close");
	err_quit("Couldn't close socket fd...%d\n");
}
int host_order(int i){
	return ntohl(i);
}
void print_addr_info(char* msg, struct addrinfo *info){
	struct sockaddr_in *sockaddr = (struct sockaddr_in *)info->ai_addr;
	char buf[512];
	const char* addr = inet_ntop(AF_INET, &sockaddr->sin_addr, buf, 512);
	if(addr == NULL){
		printf("Something went wrong... no host address\n");
		perror("inet_ntop");
	}
	printf("%s %s:%d\n",msg,addr, ntohs(sockaddr->sin_port));
}
dds_sock make_dds_socket(){
	printf("Making socket...\n");
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1){
		perror("socket");
		err_quit("Falied to create a dds_socket\n");
	}
	dds_sock sock = (_dds_sock*)malloc(sizeof(_dds_sock));
	sock->bytes = 0;
	sock->data = NULL;
	sock->msgs = 0;
	sock->fd = fd;
	printf("Done making socket %d ...\n", sock->fd);
	return sock;
}
/*
dds_sock make_local_dds_socket(void){
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd == -1){
		perror("socket");
		err_quit("Local socket creation failed");
	}
	dds_sock sock = malloc(sizeof(_dds_sock));
	sock->bytes = 0;
	sock->data = NULL;
	sock->msgs = 0;
	sock->fd = fd;
	return sock;
}

dds_sock get_local_connection(void){
	int err;
	dds_sock sock;
	struct sockaddr_un address;
	sock = make_local_dds_socket();
	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, "dds_local_socket");
	int addr_size = sizeof(address);
	err = connect(sock->fd, (struct sockaddr *)&address, addr_size);
	if(err == -1){
		perror("Could not get local connection");
		return NULL;
	}
	return sock;
}

dds_sock make_local_connection(void){
	struct sockaddr_un sockmain;
	struct sockaddr_un sockpymod;
	dds_sock sock = make_local_dds_socket();
	unlink("dds_local_socket");
	sockmain.sun_family = AF_UNIX;
	strcpy(sockmain.sun_path, "dds_local_socket");
	int servlen = sizeof(sockmain);
	bind(sock->fd, (struct sockaddr*)&sockmain, servlen);
	listen(sockmain, 3);


}
*/
#define DDS_PIPE "../build/.dds_pipe"
int setup_pipe(void){
	int fd;
	mkfifo(DDS_PIPE, S_IFIFO | 0666);
	fd = open(DDS_PIPE, O_WRONLY | O_NDELAY);
	return fd;
}
void write_to_pipe(char *to_write){
	int num;
	int fd = setup_pipe();
	if(fd == -1){ return; }
	if((num = write(fd,to_write,strlen(to_write))) == -1){
		perror("write");
	}
}
int open_pipe(void){
	int fd;
	//mkfifo(DDS_PIPE, S_IFIFO | 0666);
	fd = open(DDS_PIPE, O_RDONLY | O_NDELAY);
	return fd;
}
char *read_from_pipe(void){
	char raw[4098];
	int fd = open_pipe();
	if(fd == -1){ return NULL; }
	int num = read(fd, raw, 4098);
	do {
		if(num == -1){
			perror("read");
			return NULL;
		}
		else {
			if(num == 0){ return NULL; }
			raw[num] = '\0';
			char *ret = malloc(num + 1);
			strcpy(ret,raw);
			printf("read_from_pipe done:\nraw: %s\nret: %s\n",raw,ret);
			return ret;
		}
	} while (num > 0);
	return NULL;
}
void *register_listener(void *arg){
	dds_sock listener = (dds_sock) arg;
	char buf[4098];
	listener->fd = open(DDS_PIPE, O_RDONLY);
	while(1){
		int num = read(listener->fd, buf, 4098);
		do {
			if(num == -1){
				perror("read");
				return NULL;
			}
			else {
				if(num!=0){
					buf[num] = '\v';
					char *resized = (char*)malloc(listener->bytes + 1 + num);
					memcpy(resized, listener->data, listener->bytes);
					memcpy(resized + listener->bytes, buf, num + 1);
					free(listener->data);
					listener->data = resized;
					listener->bytes += num + 1;
					listener->msgs = 0;
					int i = 0;
					for(;i < listener->bytes; ++i){
						if(resized[i] == '\v')
							(listener->msgs)++;
					}
					memset(buf,0,sizeof(buf));
					num = 0;
				}
			}
		} while (num > 0);
	}
	return NULL;
}
pthread_t listener_thread;
dds_sock plugin_listener(void){
	dds_sock ret = make_dds_socket();
	pthread_create(&listener_thread, NULL, register_listener, ret);
	return ret;
}
dds_sock open_connection(char* addr, char* port){
	printf("Attempting to connect to %s:%s\n", addr, port);
	int err;
	dds_sock sock;
	struct addrinfo *index;
	struct sockaddr_in *sockaddr;
	struct addrinfo hint_struct;
	struct addrinfo *addrlist;
	struct addrinfo *localaddr;
	hint_struct.ai_flags = 0;
	hint_struct.ai_family = AF_INET;
	hint_struct.ai_socktype = SOCK_STREAM;
	hint_struct.ai_protocol = 0;
	hint_struct.ai_addrlen = 0;
	hint_struct.ai_canonname = NULL;
	hint_struct.ai_addr = NULL;
	hint_struct.ai_next = NULL;

	sock = make_dds_socket();

	err = getaddrinfo(addr, port, &hint_struct, &addrlist);
	if(err!=0){
		close_connection(sock);
		err_quit(gai_strerror(err));
	}

	err = getaddrinfo(addr, port, &hint_struct, &localaddr);
	if(err!=0){
		close_connection(sock);
		err_quit(gai_strerror(err));
	}
	//print_addr_info("Attempting to bind socket to ", localaddr);
	//err = bind(sock->fd, localaddr->ai_addr, localaddr->ai_addrlen);
	//if(err == -1){
	//	perror("bind");
	//	close_connection(sock);
	//	err_quit("socket bind failed ...\n");
	//}


	err = connect(sock->fd, addrlist->ai_addr, addrlist->ai_addrlen);
	if(err == -1){
		perror("connect");
		close_connection(sock);
		err_quit("Socket connect failed...\n");
	}
	print_addr_info("Connected to host: ", addrlist);	
	//TO free everything
	freeaddrinfo(addrlist);
	return sock;
}

