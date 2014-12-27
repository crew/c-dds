#include <pthread.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/shm.h>

#include "dds_io.h"
#include "dds_gtk.h"
#include "dict.h"
#include "readcfg.h"
#include "dds_gtk.h"
#include "dds_sem.h"
#define KEY_PATH "/home/pi/c-dds/src/main.c"



//Should be assigned a shared memory address
char* url_to_display;
WebKitWebView* view;
dds_sem lock;
gboolean gtk_update_page(void* args){
	char* arg = (char*)args;
	try_dds_sem(lock);
	if(strcmp(url_to_display, arg)){
		webkit_web_view_load_uri(view, url_to_display);
		strcpy(url_to_display, arg);

	}
	release_dds_sem(lock);
	return TRUE;
}
int main(int argc, char** argv){		
	Dict* d = readConfig("../Configs/PIE.conf");
	key_t key = ftok(KEY_PATH, 's');
	if(key == -1){
		perror("ftok");
		exit(1);
	}
	int shmid = shmget(key, 1024, 0666 | IPC_CREAT);
	if(shmid == -1){
		perror("shmget");
		exit(1);
	}
	url_to_display = (char*)shmat(shmid, (void*)0, 0);
	
	lock = dds_open_sem("/dds_gtk_sem", 1);
	strcpy(url_to_display,(char*)dict_get_val(d, "init_page"));
	printf("Initial display is %s\n",url_to_display);
	view = make_view(url_to_display);
	if(!fork()){
		char arg[1024];
		arg[0] = '\0';
		g_timeout_add(1000,(GSourceFunc) gtk_update_page, (void*)arg);
		gtk_main();
		exit(0);
		
	}else{
		char buf[1024];
		printf("Hello I'll be doing your page switching today (I also happen to be running in a different process as gtk :D)\n");

		while(1){
			printf("Please give me a url to navigate to (don't make it longer then 1024 char's)\n");
			printf("Type exit to terminate...\n");
			scanf("%s", buf);
			if(!strcmp("exit",buf)){
				break;
			}
			printf("Alright im going to switch the window to %s\n", buf);
			try_dds_sem(lock);
			strcpy(url_to_display, buf);
			release_dds_sem(lock);

		}
		printf("Alright now im waiting for gtk to exit...\n");

		wait(NULL);
	}

	
	
	
	/*dds_sock sock = open_connection(argv[1],argv[2]);
	char* str = "Hello World!\v2\v";
	printf("Sending message!\n");
	int send_return = write_sb(sock, str, strlen(str)+1);
	printf("Sent %d bytes!\n", send_return);


	printf("Attemping to recieve from socket...\n");
	int read_return = read_b(sock, 256);
	printf("Recieved %d bytes on read!\n", read_return);
	if(msg_complete(sock)){
		printf("I've got a message!\n");
	}else{
		printf("I don't have a message!\n");
	}
	printf("Socket has %d messages in it!\n", get_msg_count(sock));
	if(msg_in_progress(sock)){
		printf("There is another message in progress!\n");
		
	}else{
		printf("There is no other messages in progress!\n");
	}
	int nxt_size = get_nxt_msg_size(sock);
	printf("The next message has size %d\n", nxt_size);
	char* buf = (char*)malloc(nxt_size);
	get_msg(sock, buf);
	printf("Got message %s\n", buf);
	free(buf);
	printf("Socket now has %d messages in it\n", get_msg_count(sock));
	nxt_size = get_nxt_msg_size(sock);
	printf("The next message has size %d\n", nxt_size);
	buf = (char*)malloc(nxt_size);
	get_msg(sock, buf);
	printf("Got message %s\n", buf);
	free(buf);
	close_connection(sock);
	*/
}


