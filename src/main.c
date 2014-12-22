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
int main(void){
	dds_sock sock = open_connection("192.168.11.126","7778");
	char* str = "Hello World!";
	//ssize_t bytes = 0;
	printf("Sending message!\n");
	write_s(sock, str);
	//bytes = send(sock,str, strlen(str)+1, 0);
	printf("Message sent!\n");


	char buf[256];
	printf("Attemping to recieve from socket...\n");

	//int bytes = recv(sock, buf, sizeof(buf), 0);
	//printf("Recieved message with %d bytes\n", bytes);
	//if(bytes == -1){
//		perror("recv");
	//}
	//printf("The message was : %s\n", buf);

	close_connection(sock);
}


