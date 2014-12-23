#include <pthread.h>

#include <unistd.h>
#include "dds_io.h"
#include "dds_gtk.h"
#include "dict.h"
#include "readcfg.h"
#include "ddsmsgqueue.h"
#include "dds_gtk.h"
#define KEY_PATH "/home/pi/c-dds/src/main.c"
void* gtk_thread(void* arg){
	WebKitWebView* view = (WebKitWebView*)arg;
	sleep(10);
	printf("Switching pages...\n");
	gdk_threads_enter();
	webkit_web_view_load_uri(view, "http://facebook.com");
	gdk_threads_leave();

}
int main(int argc, char** argv){		
	dds_sock sock = open_connection(argv[1],argv[2]);
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
}


