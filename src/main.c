#include <pthread.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>
#include <Python.h>
#include "dds_io.h"
#include "dds_gtk.h"
#include "dict.h"
#include "readcfg.h"
#include "dds_gtk.h"
#include "dds_sem.h"
#include "parseJSON.h"
#include "dds_slides.h"
//#define KEY_PATH "/home/pi/c-dds/src/main.c"
#define MAX_URL_LEN 1024

typedef struct _dds_gtk_args{
	dds_sem lock;
	WebKitWebView* view;
	char* cur_url;
	char previous_url[MAX_URL_LEN];
}timeout_args;
static dds_sock global_sock;
static timeout_args* global_args;
static pid_t gtk_id;
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
static void terminate(int signal){
	kill(gtk_id, SIGKILL);
	wait(NULL);
	if(global_sock != NULL){
		close_connection(global_sock);
	}
	final_close_sem(global_args->lock);
	shmdt(global_args->cur_url);
	exit(signal);
}
static void segfault_p(){	
	printf("\n(Parent | PID %d) Segmentation fault signal received...\n",getpid());
	terminate(SIGSEGV);
}
static void interrupt_p(){
	printf("\n(Parent | PID %d) Interrupt signal received...\n",getpid());
	terminate(SIGINT);

}
static void interrupt_c(){
	printf("\n(Child  | PID %d) Interrupt signal received...\n",getpid());
	close_dds_sem(global_args->lock);
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


	global_sock = NULL;
	gtk_id = fork();
	if(gtk_id == 0){
		signal(SIGINT, interrupt_c);
		g_timeout_add(1000,(GSourceFunc) gtk_update_page, (void*)NULL);
		gtk_main();
		exit(0);
		
	}else{
		signal(SIGINT, interrupt_p);
		signal(SIGSEGV, segfault_p);
		slide_list slides = make_list((char*)dict_get_val(config, "init_page"), atoi((char*)dict_get_val(config, "init_duration")), -1);
		 
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
		//Main loop
		time_t start_measure;
		time(&start_measure);

		while(1){
			time_t cur_time;
			time(&cur_time);
			slide* cur_slide = get_current_slide(slides);
			if(cur_time - start_measure >  cur_slide->dur){
				printf("Switching slide...\n");
				advance_list_index(slides);
				cur_slide = get_current_slide(slides);
				try_dds_sem(lock);
				strcpy(global_args->cur_url, cur_slide->location);
				release_dds_sem(lock);
				time(&start_measure);
			}
			//Doesn't block...
			read_db(to_server,512);
			while(get_msg_count(to_server) > 0){
				printf("I have a message...\n");
				int nxt_size = get_nxt_msg_size(to_server);
				char* msg_buf = (char*)malloc(nxt_size);

				get_msg(to_server, msg_buf);
				printf("Got message %s\n", msg_buf);
				socket_message* p_msg = json_to_message(msg_buf);
				free(msg_buf);
				printf("Got message with action %d\n", p_msg->action);
				SLIDE_ACTION action = p_msg->action;
				if(action == LOAD_SLIDES){
					printf("Loading new slides!\n");
					socket_message_content* content = p_msg->content;
					action_data** ad = content->actions;
					int i = 0;
					for(;i < content->num_actions;i++){
						action_data* data = ad[i];
						if(data->type == ADT_SLIDE){
							slide_action_info* slide_info = data->slide_data;
							printf("Loading slide [%s,%d,%d]\n",slide_info->location, slide_info->duration, slide_info->id);
							add_slide(slides,make_slide( slide_info->location, slide_info->duration, slide_info->id));
						}
					}
					delete_slide_with_id(slides, -1);
				//TODO need to free message?
				}else if(action == ADD_SLIDE){
					printf("Adding new slide!\n");
					socket_message_content* c = p_msg->content;
					Dict* d = c->meta;
					Dict* id_d = simple_get_val(d, "ID");
					Dict* link_d = simple_get_val(d, "permalink");
					Dict* dur_d = simple_get_val(d, "duration");
					if(!id_d){
						printf("Id was null ...\n");
						continue;
					}
					if(!dur_d){
						printf("Dur was null...\n");
						continue;
					}
					if(!link_d){
						printf("Link was null...\n");
						continue;
					}
					printf("Adding slide [%s,%d,%d]\n",link_d->value, id_d->value, dur_d->value);
					add_slide(slides,make_slide(link_d->value, *((int*)dur_d->value), *((int*)id_d->value)));
				}else if(action == EDIT_SLIDE){
					printf("Editing a slide!\n");
					socket_message_content* c = p_msg->content;
					Dict* d = c->meta;
					Dict* id_m = (Dict*)simple_get_val(d, "ID");
					Dict* link_m = (Dict*)simple_get_val(d, "permalink");
					Dict* dur_m = (Dict*) simple_get_val(d, "duration");
					if(id_m == NULL){
						printf("Edit id was null...\n");
						continue;
					}
					if(link_m == NULL){
						printf("Link was null...\n");
						continue;
					}
					if(dur_m == NULL){
						printf("Dur was null...\n");
						continue;
					}
						
					int id = *((int*)id_m->value);
					int duration = *((int*)dur_m->value);
					char* loc = (char*)link_m->value;
					printf("Edditing slide with [%s,%d,%d]\n",loc, id, duration);
					set_slide_with_id(slides, loc, duration, id);
					printf("Done editting the slide...\n");
				}else if(action == DELETE_SLIDE){
					printf("Deleteing a slide!\n");
					Dict* d = p_msg->content->meta;
					Dict* id_m = simple_get_val(d, "ID");
					if(id_m == NULL){
						printf("ID in delete was null...\n");
						continue;
					}
					printf("Deleting slide with id %d\n",id_m->value);
					delete_slide_with_id(slides, *((int*)id_m->value));
				}else if(action == TERMINATE){
					printf("Recieved the TERMINATE action... terminating...\n");
					kill(gtk_id, SIGINT);
					wait(NULL);
					exit(0);
				}
				if(p_msg){
					printf("Deleteing the message...\n");
					delete_socket_message(p_msg);
				}
			}

		}
	}
	wait(NULL);
}


