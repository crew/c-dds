#include "dds_io.h"
void err_quit(const char* str){
	fprintf(stderr, "%s\n%s\n",str, strerror(errno));
	exit(errno);
}
int write_s(dds_sock s, const char* str, int flags){
	int tot = 0;
	int need = strlen(str)+1;
	int bytes = send(s, str, need, flags);
	if(bytes == -1){
		perror("send");
		exit(errno);
	}
	return bytes;
}
int write_sb(dds_sock s, const char* str){
	return write_s(s,str, 0);
}
int write_snb(dds_sock s, const char* str){
	return write_s(s, str, MSG_DONTWAIT);
}

int net_order(int i){
	return htonl(i);
}
int close_connection(dds_sock sock){
	int err = close(sock);
	if(!err)
		return 0;
	perror("close");
	err_quit("Couldn't close socket fd...%d\n");
}
int host_order(int i){
	return ntohl(i);
}
void print_addr_info(struct addrinfo *info){
	struct sockaddr_in *sockaddr = (struct sockaddr_in *)info->ai_addr;
	char buf[512];
	const char* addr = inet_ntop(AF_INET, &sockaddr->sin_addr, buf, 512);
	if(addr == NULL){
		printf("Something went wrong... no host address\n");
		perror("inet_ntop");
	}
	printf("Found host %s\n", addr);
}
dds_sock make_dds_socket(){
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1){
		perror("socket");
		err_quit("Falied to create a dds_socket\n");
	}
	return fd;
}

dds_sock open_connection(char* addr, char* port){
	printf("Attempting to connect to %s:%s\n", addr, port);
	int err;
	dds_sock sock;
	struct addrinfo *index;
	struct sockaddr_in *sockaddr;
	struct addrinfo hint_struct;
	struct addrinfo *addrlist;
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
		close(sock);
		err_quit(gai_strerror(err));
	}
	for(index = addrlist;index != NULL;index = index->ai_next){
		print_addr_info(index);
	}
	err = bind(sock, addrlist->ai_addr, addrlist->ai_addrlen);
	if(err == -1){
		perror("bind");
		close(sock);
		err_quit("socket bind failed ...\n");
	}



	err = connect(sock, addrlist->ai_addr, addrlist->ai_addrlen);
	if(err == -1){
		perror("connect");
		close(sock);
		err_quit("Socket connect failed...%d\n");
	}

	//TO free everything
	printf("Freeing list \n");
	freeaddrinfo(addrlist);
	
	return sock;
}

