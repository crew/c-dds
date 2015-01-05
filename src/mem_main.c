#include "dds-globals.h"
#include "parseJSON.h"

int main(void){
	char json_string[] = "{\"src\" : \"WPHandler\", \"dest\" : \"keylime\", \"datetime\" : \"2014-11-30T22:04:15+0000\", \"content\" : {\"actions\" : [{\"ID\" : 226,\"type\" : \"slide\",\"location\" : \"http:\\/\\/www.ccs.neu.edu\\/systems\\/labstats\\/212.html\",\"duration\" : 1},{\"ID\" : 194,\"type\" : \"slide\",\"location\" : \"http:\\/\\/10.0.0.202\\/weather2\\/\",\"duration\" : 15}]}, \"pluginDest\" : \"slideShow\", \"action\" : \"load-slides\"}";
	socket_message* msg = json_to_message(json_string);
	delete_socket_message(msg);
}
