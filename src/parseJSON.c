#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <syslog.h>
#include <string.h>
#include "cJSON.h"
#include "dict.h"
#include "parseJSON.h"


slide_action parse_action(char *str) {
    if (~strcmp(str, "add-slide")) {
        return ADD_SLIDE;
    }
    else if (~strcmp(str, "delete-slide")) {
        return DELETE_SLIDE;
    }
    else if (~strcmp(str, "edit-slide")) {
        return EDIT_SLIDE;
    }
    else if (~strcmp(str, "Terminate")) {
        return TERMINATE;
    }
    else if(~strcmp(str, "querySlides")){
    	return LOAD_SLIDES;
    }
    else {
        syslog(LOG_MAKEPRI(LOG_SYSLOG, LOG_ERR),
                "parse_action: Recieved an invalid slide action");
        closelog();
        return (slide_action) NULL;
    }
}

char *action_string(slide_action action) {
    if (action == ADD_SLIDE) {
        return "add-slide";
    }
    else if (action == DELETE_SLIDE) {
        return "delete-slide";
    }
    else if (action == EDIT_SLIDE) {
        return "edit-slide";
    }
    else if (action == TERMINATE) {
        return "Terminate";
    }
    else if (action == LOAD_SLIDES){
    	return "querySlides";
    }
    else {
        syslog(LOG_MAKEPRI(LOG_SYSLOG, LOG_ERR),
                "action_string: Recieved an invalid slide action");
        closelog();
        return NULL;
    }
}

pie *parse_pie(cJSON *raw_pie) {
    char *name = cJSON_GetObjectItem(raw_pie, "name")->valuestring;
    pie *to_ret = malloc(sizeof(pie) + sizeof(name));
    to_ret->name = name;
    return to_ret;
}

cJSON *pie_to_json(pie *parsed_pie) {
    cJSON *to_ret = cJSON_CreateObject();
    cJSON_AddStringToObject(to_ret, "name", parsed_pie->name);
    return to_ret;
}

void parse_pies(cJSON *pie_array, socket_message *to_set) {
    int len = cJSON_GetArraySize(pie_array);
    to_set->pie_list_len = len;
    to_set->pie_list = malloc(len * sizeof(pie));
    // Check if something went wrong
    if (to_set->pie_list == NULL) {
        syslog(LOG_MAKEPRI(LOG_SYSLOG, LOG_ERR), "Could not allocate pie array for received message");
    }
    int i;
    for (i = 0; i < len; i++) {
        to_set->pie_list[i] = parse_pie(cJSON_GetArrayItem(pie_array, i));
    }
}

cJSON *pie_arr_to_json(pie *pie_array[], int len) {
    int i;
    cJSON *ret_arr = cJSON_CreateArray();
    for (i = 0; i < len; i++) {
        cJSON_AddItemToArray(ret_arr, pie_to_json(pie_array[i]));
    }
    return ret_arr;
}

const char *parse_date(const char *input, struct tm *tm) {
    const char *cp;

    /* First clear the result structure.  */
    memset(tm, '\0', sizeof(*tm));

    /* Try the ISO format first.  */
    cp = strptime(input, "%FT%T%z", tm);
    if (cp == NULL) {
        /* Does not match.  Try the US form.  */
        cp = strptime(input, "%D", tm);
    }

    return cp;
}

void recursive_parse(cJSON *current, Dict *d, int arr_idx) {
    if (current == NULL) {return;}
    socket_meta *parsed = (socket_meta *)malloc(sizeof(socket_meta));
    if (current->child) {
        META_TYPE type;
        int nextnum;
        if (current->type == cJSON_Array) {
            parsed->type = T_ARR;
            nextnum = 0;
        }
        else if (current->type == cJSON_Object) {
            parsed->type = T_DICT;
            nextnum = -1;
        }
        Dict *ret_dict = make_dict();
        recursive_parse(current->child, ret_dict, nextnum);
        parsed->value = ret_dict;
    }
    else {
            if (current->type == cJSON_Number) {
                // TODO: Doubles
                int *val_int = malloc(sizeof(current->valueint));
                *val_int = current->valueint;
                parsed->type = T_INT;
                parsed->value = val_int;
            }
            else if (current->type == cJSON_String) {
                char *str;
                str = (char *)malloc(sizeof(char) * strlen(current->valuestring));
                strcpy(str, current->valuestring);
                parsed->type = T_POINT_CHAR;
                parsed->value = str;
            }
            else {
                void *value;
                if (current->valuestring) {
                    char *temp;
                    temp = (char *)malloc(sizeof(char) * strlen(current->valuestring));
                    strcpy(temp, current->valuestring);
                    value = (void *)temp;
                }
                else if (current->valueint) {
                    int *temp;
                    temp = (int *)malloc(sizeof(current->valueint));
                    *temp = current->valueint;
                    value = (void *)temp;
                }
                else if (current->valuedouble) {
                    double *temp;
                    temp = (double *)malloc(sizeof(current->valuedouble));
                    *temp = current->valuedouble;
                    value = (void *)temp;
                }
                parsed->type = T_POINT_VOID;
                parsed->value = value;
                //*parsed = (socket_meta) {value, T_POINT_VOID};
            }
    }
    if(current->string) {
        dict_put(d, current->string, (void *)parsed);
    }
    else{
    	char *arr_key = (char*)malloc(6 * sizeof(char)); // Limit array length to 1,000,000
    	sprintf(arr_key,"%d",arr_idx);
        dict_put(d, arr_key, (void *)parsed);
    }
    Dict dct;
    dct = *(d);
    while(dct.next){dct = *dct.next;}
    recursive_parse(current->next, d, ((arr_idx == -1) ? -1 : arr_idx + 1));
}

Dict *parse_json_meta(cJSON *raw_meta) {
    Dict *ret = make_dict();
    recursive_parse(raw_meta->child, ret, -1);
    return ret;
}

void parse_and_add_to(Dict *to_add, cJSON *add_to, int is_object) {
	if (to_add){
		if (to_add->value){
        if ((((socket_meta *) to_add->value)->type) == T_INT){
            if (is_object) {
                cJSON_AddNumberToObject(add_to, to_add->key, *((int*)((socket_meta *) to_add->value)->value));
            }
            else {
                cJSON_AddNumberToArray(add_to, *((int*)((socket_meta *) to_add->value)->value));
            }
        }
        else if ((((socket_meta *) to_add->value)->type) == T_POINT_CHAR) {
            if (is_object) {
                cJSON_AddStringToObject(add_to, to_add->key, (char *) ((socket_meta *) to_add->value)->value);
            }
            else {
                cJSON_AddStringToArray(add_to, (char *) ((socket_meta *) to_add->value)->value);
            }
        }
        else if ((((socket_meta *) to_add->value)->type) == T_ARR) {
            cJSON *temp = cJSON_CreateArray();
            parse_and_add_to((Dict *) ((socket_meta *) to_add->value)->value, temp, 0);
            if (is_object) {
                cJSON_AddItemReferenceToObject(add_to, to_add->key, temp);
            }
            else {
                cJSON_AddItemReferenceToArray(add_to, temp);
            }
        }
        else if ((((socket_meta *) to_add->value)->type) == T_DICT) {
            cJSON *temp = cJSON_CreateObject();
            parse_and_add_to((Dict *) ((socket_meta *) to_add->value)->value, temp, 1);
            if (is_object) {
                cJSON_AddItemReferenceToObject(add_to, to_add->key, temp);
            }
            else {
                cJSON_AddItemReferenceToArray(add_to, temp);
            }
        }
    parse_and_add_to(to_add->next, add_to, is_object);
		}
		else if(to_add->next){
			parse_and_add_to(to_add->next,add_to,is_object);
		}
	}
}

cJSON *parse_message_meta(Dict *msg_meta) {
    cJSON *meta;
    meta = cJSON_CreateObject();
    parse_and_add_to((Dict *) msg_meta->next, meta, 1); //next b/c first dict entry is null
    return meta;
}

socket_message *json_to_message(char *str) {
    cJSON *input, *content;
    input = cJSON_Parse(str);
    content = cJSON_GetObjectItem(input, "content");
    socket_message_content *msg_c = (socket_message_content *) malloc(sizeof(socket_message_content));
    msg_c->id = cJSON_GetObjectItem(content, "ID")->valueint;
    msg_c->permalink = cJSON_GetObjectItem(content, "Permalink")->valuestring;
    msg_c->meta = parse_json_meta(cJSON_GetObjectItem(content, "meta"));
    socket_message *msg = (socket_message *) malloc(sizeof(socket_message));
    msg->datetime = malloc(sizeof(struct tm));
    parse_date(cJSON_GetObjectItem(input, "datetime")->valuestring, msg->datetime);
    msg->action = parse_action(cJSON_GetObjectItem(input, "action")->valuestring);
    msg->content = msg_c;
    parse_pies(cJSON_GetObjectItem(input, "pies"), msg);
    return msg;
}

char *message_to_json(socket_message *msg) {

    cJSON *root, *content;
    root = cJSON_CreateObject();
    char datetime[27];
    strftime(datetime, sizeof(datetime), "%FT%T%z", msg->datetime);
    cJSON_AddStringToObject(root, "datetime", datetime);
    cJSON_AddStringToObject(root, "action", action_string(msg->action));
    cJSON_AddItemToObject(root, "pies", pie_arr_to_json(msg->pie_list, msg->pie_list_len));
    cJSON_AddItemToObject(root, "content", content = cJSON_CreateObject());
    cJSON_AddNumberToObject(content, "ID", msg->content->id);
    cJSON_AddStringToObject(content, "Permalink", msg->content->permalink);
    cJSON_AddItemToObject(content, "meta", parse_message_meta(msg->content->meta));
    return cJSON_Print(root);
}

void dump_meta_atom(socket_meta *mem, int indents);
void dump_meta_dict(Dict *d, int indents);
//void dump_meta_array(Dict *d, int indents);

void dump_meta_dict(Dict *d, int indents){
	int i = 0;
	char tabs[indents + 1];
	for(;i<indents;i++){
		tabs[i] = '\t';
	}
	tabs[indents] = '\0';
	int len = dict_size(d);
	i = 0;
	Dict *temp = d;
	for(;i<=len;i++){
		printf("%s%s\t: ",tabs,temp->key);
		dump_meta_atom((socket_meta *)temp->value, indents);
		printf((i < len-1) ? ",\n" : "\n");
		temp = temp->next;
	}
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
void dump_meta_atom(socket_meta *mem, int indents){
	int i = 0;
	char tabs[indents + 1];
	for(;i<indents;i++){
		tabs[i] = '\t';
	}
	tabs[indents] = '\0';
	if(mem->type == T_INT){
		printf("%d",*(int *)mem->value);
	}
	else if(mem->type == T_CHAR){
		printf("%c",*(char *)mem->value);
	}
	else if(mem->type == T_POINT_INT){
		printf("%d (0x%x)", *(int *)mem->value, (int)mem->value);
	}
	else if(mem->type == T_POINT_CHAR){
		printf("%s (0x%x)", (char *)mem->value, (int)mem->value);
	}
	else if(mem->type == T_POINT_VOID){
		printf("<void pointer>");
	}
	else if(mem->type == T_ARR){
		printf("Array: [\n");
		dump_meta_dict(((Dict*)mem->value)->next, indents + 1);
		printf("%s\t\t]",tabs);
	}
	else if(mem->type == T_DICT){
		printf("Object: {\n");
		dump_meta_dict(((Dict*)mem->value)->next, indents + 1);
		printf("%s\t\t}",tabs);
	}
}
#pragma GCC diagnostic pop

void dump_meta(Dict *meta){
	printf("meta: {\n");
	dump_meta_dict(meta->next, 1);
	printf("}\n");
}

Dict *meta_to_dict(Dict *meta){
	if (meta == NULL){return NULL;}
	Dict *to_ret = malloc(sizeof(*meta));
	memcpy(to_ret,meta,sizeof(*meta));
	if(to_ret->value != NULL){
		if ((((socket_meta*)to_ret->value)->type == T_ARR) || (((socket_meta*)to_ret->value)->type == T_DICT)){
			to_ret->value = meta_to_dict((Dict *)((socket_meta*)meta->value)->value);
		}
		else{
			to_ret->value = ((socket_meta*)to_ret->value)->value;
		}
	}
	to_ret->next = meta_to_dict(meta->next);

}
/*
//Testing function...compile with gcc parseJSON.c cJSON.c dict.c -lm -lrt
char json_string[] = "{\"datetime\" : \"2014-11-30T22:04:15+0000\",\"action\" : \"add-slide\",\"pies\" : [{ \"name\" : \"shepard\" },{ \"name\" : \"blueberry\" }],\"content\"  : {\"ID\" : 14,\"Permalink\" : \"http://dds-wp...\", \"meta\" : {\"key1\" : [ \"value\" ],\"key2\" : [\"value1\",\"value2\",3,{\"meta can be weird\" : \"remember that\"}]}}}";

int main() {
    socket_message *sm = json_to_message(json_string);
    #define DT_BUF_SIZE 27
    char dt[ DT_BUF_SIZE ];
    strftime(dt, sizeof(dt), "%Y-%m-%dT%H:%M:%S%z", sm->datetime);
    printf("sm:\ndatetime:%s\naction:%s\npies:\n\t(name: %s)\n\t(name: %s)\ncontent:\n\tid: %d\n...\n",
            dt, action_string(sm->action), sm->pie_list[0]->name, sm->pie_list[1]->name, sm->content->id);
    //printf(dump_dict(sm->content->meta));
    dump_meta(sm->content->meta);
    //printf("sm->content->meta->key2->2: %d\n", *(int*)dict_get_val(meta_to_dict(sm->content->meta),"key2","2"));
    //printf(dump_dict(((Dict *)(sm->content->meta->value))->next->next->next->value));
    printf("back again:\n%s\n", message_to_json(sm));
    //printf("sm:\naction:%s\npies:\n\t(name: %s)\n\t(name: %s)\ncontent:\n\tid: %d\n...\n", action_string(sm->action), sm->pie_list[0]->name, sm->pie_list[1]->name, sm->content->id);
}
*/






