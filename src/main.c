#include <pthread.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>
#include "dds_io.h"
#include "dds_gtk.h"
#include "dict.h"
#include "readcfg.h"
#include "dds_gtk.h"
#include "dds_sem.h"
#include "parseJSON.h"
#include "dds_slides.h"
#define KEY_PATH "/home/pi/c-dds/src/main.c"
#define MAX_URL_LEN 1024

typedef struct _dds_gtk_args{
	dds_sem lock;
	WebKitWebView* view;
	char* cur_url;
	char previous_url[MAX_URL_LEN];
}timeout_args;
static dds_sock global_sock;
static timeout_args* global_args;
gboolean gtk_update_page(void* arg_void){
	try_dds_sem(global_args->lock);
	if(strcmp(global_args->cur_url, global_args->previous_url)){
		webkit_web_view_load_uri(global_args->view, global_args->cur_url);
		if(strlen(global_args->cur_url) > MAX_URL_LEN){
			global_args->cur_url[MAX_URL_LEN] = '\0';	
		}
		strcpy(global_args->previous_url, global_args->cur_url);
	}
	release_dds_sem(global_args->lock);
	return TRUE;
}
void interrupt(){
	printf("Interrupt signal recieved...\n");
	if(global_sock != NULL){
		close_connection(global_sock);
	}
	final_close_sem(global_args->lock);
	shmdt(global_args->cur_url);
	exit(SIGINT);
}
void* make_shmmem(){
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
//	shmctl(shmid, IPC_RMID, 0);
	return shmat(shmid, (void*)0, 0);
}
static struct tm* get_cur_tm(){
	time_t t;
	time(&t);
	return localtime(&t);
}
static char* get_load_slide_msg(Dict* config){
	
	socket_message load_slide_msg;
	load_slide_msg.datetime = get_cur_tm();
	load_slide_msg.action = LOAD_SLIDES;
	load_slide_msg.plugin_dest = "WPHandler";
	socket_message_content content;
	content.meta = NULL;
	content.num_actions = 0;
	content.actions = NULL;
	load_slide_msg.content = &content;
	pie src, dest;
	src.name = dict_get_val(config, "name");
	dest.name = "Grandma";
	load_slide_msg.src = &src;
	load_slide_msg.dest = &dest;
	char* msg_string = message_to_json(&load_slide_msg);
	return msg_string;
}
static char* get_hello_msg(Dict* config){
	socket_message hello_msg;
	hello_msg.datetime = get_cur_tm();
	hello_msg.action = CONNECT;
	hello_msg.plugin_dest = "socketServer";
	socket_message_content helcon;
	helcon.meta = make_dict();
	dict_put(helcon.meta, "name", (char*)dict_get_val(config, "name"));
	helcon.num_actions = 0;
	helcon.actions = NULL;
	hello_msg.content = &helcon;
	pie hsrc, hdest;
	hsrc.name = dict_get_val(config, "name");
	hdest.name = "Grandma";
	hello_msg.src = &hsrc;
	hello_msg.dest = &hdest;
	char* hello = message_to_json(&hello_msg);
	return hello;
}
//TODO connect termination signal to free ALL the shit
int main(int argc, char** argv){
	dds_sem lock = dds_open_sem("/dds_gtk_sem", 1);	
	Dict* config = readConfig("../Configs/PIE.conf");
	timeout_args targs;
	global_args = &targs;
 	global_args->cur_url = (char*)make_shmmem();
	global_args->previous_url[0] = '\0';
	global_args->lock = lock;
	strcpy(global_args->cur_url,(char*)dict_get_val(config, "init_page"));
	global_args->view = make_view(global_args->cur_url);
	printf("Initial display is %s\n", global_args->cur_url);

	signal(SIGINT, interrupt);
	global_sock = NULL;
	if(!fork()){
		g_timeout_add(1000,(GSourceFunc) gtk_update_page, (void*)NULL);
		gtk_main();
		exit(0);
		
	}else{
		char* url = dict_get_val(config, "server");
		char* port = dict_get_val(config, "port");

		dds_sock to_server = global_sock = open_connection(url, port);
		
		//Saying hello to the server
		char* init_msg = get_hello_msg(config);
		write_sb(to_server, init_msg, strlen(init_msg));
		printf("Wrote hello message...\n");
		free(init_msg);
		//TODO python server needs to do \v splitting...

		char* load_slides_msg = get_load_slide_msg(config);
		int wrote = write_sb(to_server, load_slides_msg, strlen(load_slides_msg));
		printf("Wrote string to server, with bytes : %d\n", wrote);
		free(load_slides_msg);

		while(get_msg_count(to_server)<1){
			read_b(to_server, 512);
		}
		int nxt_size = get_nxt_msg_size(to_server);

		char* buf = (char*)malloc(nxt_size);
		get_msg(to_server, buf);
		printf("Recieved message %s with len %d\n", buf, strlen(buf));
		if(nxt_size != strlen(buf)+1){
			printf("Something went horribly wrong...\n");
		}
		socket_message* msg = json_to_message(buf);
		free(buf);

		
	}
	wait(NULL);


	
	
	
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


