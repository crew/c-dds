
#include <unistd.h>
#include "dds_gtk.h"
#include "dict.h"
#include "readcfg.h"
#include "ddsmsgqueue.h"
#include "dds_gtk.h"
#define KEY_PATH "/home/pi/c-dds/src/main.c"

int main(void){
	int qid = get_q_id(KEY_PATH, 'd');
	Dict* d = readConfig("../Configs/PIE.conf");

	WebKitWebView* view = make_view((char*)dict_get_val(d, "init_page"));	
	if(!fork()){
		gtk_main();
	}



	destroy_q(qid);
	//Deallocates the dictionary
	delete_dict_and_contents(d);
	printf("Main Process is waiting...\n");
	webkit_web_view_load_uri(view, "http://facebook.com");
	wait(NULL);
}


