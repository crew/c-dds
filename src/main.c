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
#define MAX_URL_LEN 1024

typedef struct _dds_gtk_args{
	dds_sem lock;
	WebKitWebView* view;
	char* cur_url;
	char previous_url[MAX_URL_LEN];
}timeout_args;

gboolean gtk_update_page(void* arg_void){
	timeout_args* arg_struct = (timeout_args*)arg_void;
	try_dds_sem(arg_struct->lock);
	if(strcmp(arg_struct->cur_url, arg_struct->previous_url)){
		webkit_web_view_load_uri(arg_struct->view, arg_struct->cur_url);
		if(strlen(arg_struct->cur_url) > MAX_URL_LEN){
			arg_struct->cur_url[MAX_URL_LEN] = '\0';	
		}
		strcpy(arg_struct->previous_url, arg_struct->cur_url);
	}
	release_dds_sem(arg_struct->lock);
	return TRUE;
}
void* make_shmmem(timeout_args* t){
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
	return shmat(shmid, (void*)0, 0);
}
//TODO connect termination signal to free ALL the shit
int main(int argc, char** argv){
	dds_sem lock = dds_open_sem("/dds_gtk_sem", 1);	
	Dict* d = readConfig("../Configs/PIE.conf");
	timeout_args *targs = (timeout_args*)malloc(sizeof(timeout_args));
	targs->cur_url= (char*)make_shmmem(targs);
	targs->previous_url[0] = '\0';
	targs->lock = lock;
	strcpy(targs->cur_url,(char*)dict_get_val(d, "init_page"));
	targs->view = make_view(targs->cur_url);
	printf("Initial display is %s\n", targs->cur_url);
	if(!fork()){
		g_timeout_add(1000,(GSourceFunc) gtk_update_page, (void*)targs);
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
			strcpy(targs->cur_url, buf);
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


