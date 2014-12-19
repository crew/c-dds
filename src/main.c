#include <pthread.h>

#include <unistd.h>
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
	pthread_t gtk_main_thread;
	Dict* d = readConfig("../Configs/PIE.conf");

	WebKitWebView* view = make_view((char*)dict_get_val(d, "init_page"));	
	pthread_create(&gtk_main_thread, NULL, gtk_thread, (void*) view);
	gtk_main();
	//webkit_web_view_load_uri(view, "http://facebook.com");
	//printf("Right before the sleep...\n");
	//sleep(10);
	//printf("Right after the sleep...\n");
	//Deallocates the dictionary
	delete_dict_and_contents(d);

}


