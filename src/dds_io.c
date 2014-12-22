#include "dds_io.h"
void err_quit(const char* str){
	fprintf(stderr, "%s\n%s\n",str, strerror(errno));
	exit(errno);
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
	struct sockaddr *sockaddr = info->ai_addr;
	char buf[512];
	const char* addr = inet_ntop(AF_INET, &info->ai_addr->sa_data, buf, 512);
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

	print_addr_info(addrlist);

	err = bind(sock, addrlist->ai_addr, addrlist->ai_addrlen);
	if(err == -1){
		perror("bind");
		close(sock);
		err_quit("socket bind failed ...%d\n");
	}



	err = connect(sock, addrlist->ai_addr, addrlist->ai_addrlen);
	if(err == -1){
		perror("connect");
		close(sock);
		err_quit("Socket connect failed...%d\n");
	}

	//TO free everything
	for(index = addrlist;index != NULL; index = index->ai_next){
		freeaddrinfo(index);
	}
	return sock;
}

