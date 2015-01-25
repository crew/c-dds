#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <syslog.h>
#include <string.h>
#include <stdbool.h>
#include "dds_globals.h"
#include "cJSON.h"
#include "dict.h"
#include "parseJSON.h"


SLIDE_ACTION parse_action(char *str) {
    
    if (!strcmp(str, "add-slide")) {
        return ADD_SLIDE;
    }
    else if (!strcmp(str, "delete-slide")) {
        return DELETE_SLIDE;
    }
    else if (!strcmp(str, "edit-slide")) {
        return EDIT_SLIDE;
    }
    else if (!strcmp(str, "Terminate")) {
        return TERMINATE;
    }
    else if(!strcmp(str, "load-slides")){
    	return LOAD_SLIDES;
    }
    else if(!strcmp(str, "connect")){
    	return CONNECT;
    }
    else {
        syslog(LOG_MAKEPRI(LOG_SYSLOG, LOG_ERR),
                "parse_action: Received an invalid slide action");
        closelog();
        return (SLIDE_ACTION) NULL;
    }
}

char *action_string(SLIDE_ACTION action) {
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
    	return "load-slides";
    }
    else if(action == CONNECT){
    	return "connect";
    }
    else {
        syslog(LOG_MAKEPRI(LOG_SYSLOG, LOG_ERR),
                "action_string: Recieved an invalid slide action");
        closelog();
        return NULL;
    }
}

pie *parse_pie(cJSON *raw_pie) {
	char *name = raw_pie->valuestring;
    pie *to_ret = malloc(sizeof(pie) + sizeof(name));
    to_ret->name = name;
    return to_ret;
}

pie *parse_pie_dict_val(void *raw_pie) {
	char *name = DYN_STR((char*)raw_pie);
    pie *to_ret = malloc(sizeof(pie) + sizeof(name));
    to_ret->name = name;
    return to_ret;
}

// In case we need to do anything fancy down the road
char *pie_to_json(pie *parsed_pie) {
    return parsed_pie->name;
}

char *pie_to_dict_val(pie *parsed_pie) {
    return parsed_pie->name;
}

slide_action_info *make_slide_action_info(int id, char *location, int duration){
	slide_action_info *to_ret = malloc(sizeof(slide_action_info));
	to_ret->id = id;
	to_ret->location = location;
	to_ret->duration = duration;
	return to_ret;
}

action_data *parse_action_data(cJSON *raw_action_data){
	action_data *to_ret = malloc(sizeof(action_data));
	char *action_type = cJSON_GetObjectItem(raw_action_data, "type")->valuestring;
	if(!strcmp(action_type,"slide")){
		to_ret->type = ADT_SLIDE;
		to_ret->slide_data = make_slide_action_info(
				cJSON_GetObjectItem(raw_action_data, "ID")->valueint,
				cJSON_GetObjectItem(raw_action_data, "location")->valuestring,
				cJSON_GetObjectItem(raw_action_data, "duration")->valueint
		);
		return to_ret;
	}
	printf("parse_action_data: WARNING: Given unimplemented slide action. Returning Null Pointer.\n");
	return NULL;
}

cJSON *action_data_to_json(action_data *to_parse){
	cJSON *to_ret = cJSON_CreateObject();
	switch(to_parse->type){
	case ADT_SLIDE:
		cJSON_AddStringToObject(to_ret, "type", "slide");
		cJSON_AddNumberToObject(to_ret, "ID", to_parse->slide_data->id);
		cJSON_AddStringToObject(to_ret, "location", to_parse->slide_data->location);
		cJSON_AddNumberToObject(to_ret, "duration", to_parse->slide_data->duration);
		break;
	default:
		break;
	}
	return to_ret;
}

action_data *parse_dict_action_data(Dict *raw_action_data){
	action_data *to_ret = malloc(sizeof(action_data));
	char *action_type = (char*)dict_get_val(raw_action_data,"type");//cJSON_GetObjectItem(raw_action_data, "type")->valuestring;
	if(!strcmp(action_type,"slide")){
		to_ret->type = ADT_SLIDE;
		to_ret->slide_data = make_slide_action_info(
				*(int*)dict_get_val(raw_action_data,"ID"),
				DYN_STR((char*)dict_get_val(raw_action_data,"location")),
				*(int*)dict_get_val(raw_action_data,"duration")
		);
		return to_ret;
	}
	printf("parse_action_data: WARNING: Given unimplemented slide action. Returning Null Pointer.\n");
	return NULL;
}

Dict *action_data_to_dict(action_data *to_parse){
	Dict *to_ret = make_dict();
	switch(to_parse->type){
	case ADT_SLIDE:
		dict_put(to_ret, DYN_STR("type"), DYN_STR("slide"));
		dict_put(to_ret, DYN_STR("ID"), DYN_NON_POINT(to_parse->slide_data->id));
		dict_put(to_ret, DYN_STR("location"), DYN_STR(to_parse->slide_data->location));
		dict_put(to_ret, DYN_STR("duration"), DYN_NON_POINT(to_parse->slide_data->duration));
		break;
	default:
		break;
	}
	return to_ret;
}

void parse_actions(cJSON *raw_actions, socket_message_content *receiver){
	int len = cJSON_GetArraySize(raw_actions);
	receiver->num_actions = len;
	receiver->actions = malloc(len * sizeof(action_data *));
	// Check if something went wrong
	if (receiver->actions == NULL) {
	    syslog(LOG_MAKEPRI(LOG_SYSLOG, LOG_ERR), "parse_actions: Could not allocate actions array for received message's content\n");
	    return;
	}
	int i;
	for(i = 0; i < len; i++){
		receiver->actions[i] = parse_action_data(cJSON_GetArrayItem(raw_actions, i));
	}
	return;
}

void parse_actions_dict(Dict *raw_actions, socket_message_content *receiver){
	if(raw_actions){
		
		int len = dict_size(raw_actions);
		receiver->num_actions = len;
		receiver->actions = malloc(len * sizeof(action_data *));
		// Check if something went wrong
		if (receiver->actions == NULL) {
		    syslog(LOG_MAKEPRI(LOG_SYSLOG, LOG_ERR), "parse_actions: Could not allocate actions array for received message's content\n");
		    return;
		}
		int i;
		char *key = malloc(6 * sizeof(char));
		for(i = 0; i < len; i++){
			sprintf(key,"%d",i);
			receiver->actions[i] = parse_dict_action_data((Dict*)dict_get_val(raw_actions,key));
		}
		free(key);
	}
	return;
}

cJSON *actions_to_json(action_data *actions[], int num_actions){
	cJSON *to_ret = cJSON_CreateArray();
	int i;
	for(i = 0; i < num_actions; i ++){
		cJSON_AddItemToArray(to_ret, action_data_to_json(actions[i]));
	}
	return to_ret;
}

Dict *actions_to_dict(action_data *actions[], int num_actions){
	Dict *to_ret = make_dict();
	int i;
	char *key = malloc(6 * sizeof(char));
	for(i = 0; i < num_actions; i ++){
		sprintf(key,"%d",i);
		dict_put(to_ret, DYN_STR(key),action_data_to_dict(actions[i]));
	}
	free(key);
	return to_ret;
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
            	if ((double)(current->valueint) == current->valuedouble){
            		int *val_int = malloc(sizeof(current->valueint));
                	*val_int = current->valueint;
                	parsed->type = T_INT;
                	parsed->value = val_int;
            	}
            	else{
            		float *val_doub = malloc(sizeof(current->valuedouble));
            		*val_doub = current->valuedouble;
            		parsed->type = T_DOUBLE;
            		parsed->value = val_doub;
            	}
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
            }
    }
    if(current->string) {
        dict_put(d, DYN_STR(current->string), (void *)parsed);
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
        else if ((((socket_meta *) to_add->value)->type) == T_DOUBLE){
            if (is_object) {
                cJSON_AddNumberToObject(add_to, to_add->key, *((double*)((socket_meta *) to_add->value)->value));
            }
            else {
                cJSON_AddNumberToArray(add_to, *((double*)((socket_meta *) to_add->value)->value));
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

/*
#define cJSON_False 0
#define cJSON_True 1
#define cJSON_NULL 2
#define cJSON_Number 3
#define cJSON_String 4
#define cJSON_Array 5
#define cJSON_Object 6
*/
Dict *cJSON_to_dict(cJSON *raw_cJSON){
	Dict *dct = make_dict();
	cJSON *cur = raw_cJSON;
	_Bool is_array = !cur->string;
	int arr_idx = 0;
	while(cur != NULL){
		//char **arr_key = malloc(sizeof(char*));
		char buf[256];
		buf[255] = '\0';
		//TODO fix this gross crap...
		char* tmp = buf;
		char** arr_key = &tmp;
		if (is_array){
			//*arr_key = (char*)malloc(6 * sizeof(char)); // Limit array length to 1,000,000
			sprintf(buf,"%d",arr_idx);
			arr_idx++;
		}
		else{

			 //*arr_key = DYN_STR(cur->string);
			 strcpy(buf, cur->string);
		}
		if((cur->type == cJSON_Array) || (cur->type == cJSON_Object)){
			Dict *d = cJSON_to_dict(cur->child);
			
			dict_put(dct, DYN_STR(*arr_key), d);
			//DICT_PUT(dct, arr_key, cJSON_to_dict(cur->child), T_ARR);
			if(cur->type == cJSON_Array){dict_override_type(dct,T_ARR,*arr_key);}
		}
		else if(cur->type == cJSON_NULL){
			dict_put(dct,DYN_STR(*arr_key),NULL);
		}
		else{
			if(cur->type == cJSON_String){
				dict_put(dct, DYN_STR(*arr_key), DYN_STR(cur->valuestring));
			}
			else if(cur->type == cJSON_Number && ((double)cur->valueint != cur->valuedouble)){
				double *to_put = malloc(sizeof(cur->valuedouble));
				*to_put = cur->valuedouble;
				dict_put(dct, DYN_STR(*arr_key), to_put);
			}
			else{
				int *to_put = malloc(sizeof(cur->valueint));
				*to_put = cur->valueint;
				dict_put(dct, DYN_STR(*arr_key), to_put);
			}
		}
		cur = cur->next;
		//free(arr_key); //TODO why does this work?
	}
	return dct;
}
// T_INT, T_DOUBLE, T_CHAR, T_POINT_INT, T_POINT_DOUBLE, T_POINT_CHAR, T_POINT_VOID, T_ARR, T_DICT
void recursive_dict_to_cJSON(cJSON *add_to, Dict *d, _Bool adding_to_array){
	if (adding_to_array){
		switch(d->type){
		case T_INT:
			cJSON_AddNumberToArray(add_to,*(int *)d->value);
			break;
		case T_DOUBLE:
			cJSON_AddNumberToArray(add_to,*(double *)d->value);
			break;
		case T_CHAR:;
			char* to_add = (char*)malloc(2);
            to_add[0] = *(char *)d->value;
            to_add[1] = '\0';
			cJSON_AddStringToArray(add_to,to_add);
			break;
		case T_POINT_CHAR:
			cJSON_AddStringToArray(add_to,DYN_STR((char*)d->value));
			break;
		case T_POINT_INT:
			cJSON_AddNumberToArray(add_to,*(int*)d->value);
			break;
		case T_POINT_DOUBLE:
			cJSON_AddNumberToArray(add_to,*(double*)d->value);
			break;
		case T_NULL:
			cJSON_AddNullToArray(add_to);
			break;
		case T_ARR:;
			cJSON *array_to_add = cJSON_CreateArray();
			recursive_dict_to_cJSON(array_to_add, ((Dict*)d->value)->next, 1);
			cJSON_AddItemToArray(add_to, array_to_add);
			break;
		case T_DICT:;
			cJSON *dict_to_add = cJSON_CreateObject();
			recursive_dict_to_cJSON(dict_to_add, ((Dict*)d->value)->next, 0);
			cJSON_AddItemToArray(add_to, dict_to_add);
			break;
		default:
			break;
		}
	}
	else{
		switch(d->type){
		case T_INT:
			cJSON_AddNumberToObject(add_to,d->key, *(int *)d->value);
			break;
		case T_DOUBLE:
			cJSON_AddNumberToObject(add_to,d->key, *(double *)d->value);
			break;
		case T_CHAR:;
			char to_add[] = {*(char *)d->value, '\0'};
			cJSON_AddStringToObject(add_to,d->key, DYN_STR(to_add));
			break;
		case T_POINT_CHAR:
			cJSON_AddStringToObject(add_to,d->key, DYN_STR((char*)d->value));
			break;
		case T_POINT_INT:
			cJSON_AddNumberToObject(add_to,d->key, *(int*)d->value);
			break;
		case T_POINT_DOUBLE:
			cJSON_AddNumberToObject(add_to,d->key, *(double*)d->value);
			break;
		case T_NULL:
			cJSON_AddNullToObject(add_to,d->key);
			break;
		case T_ARR:;
			cJSON *array_to_add = cJSON_CreateArray();
			recursive_dict_to_cJSON(array_to_add, ((Dict*)d->value)->next, 1);
			cJSON_AddItemToObject(add_to, d->key, array_to_add);
			break;
		case T_DICT:;
			cJSON *dict_to_add = cJSON_CreateObject();
			recursive_dict_to_cJSON(dict_to_add, ((Dict*)d->value)->next, 0);
			cJSON_AddItemToObject(add_to, d->key, dict_to_add);
			break;
		default:
			break;
		}
	}
	if(d->next){return recursive_dict_to_cJSON(add_to, d->next, adding_to_array);}
	return;
}

cJSON *dict_to_cJSON(Dict *d){
	cJSON *ret = cJSON_CreateObject();
	recursive_dict_to_cJSON(ret,d->next,0);
	return ret;
}

short socket_message_pred(cJSON *msg){
    // Most reliable difference, for now
    if(cJSON_GetObjectItem(msg,"action")){
        return 1;
    }
    return 0;
}

socket_message *json_to_socket_message(cJSON *input) {
    cJSON *content;
    //input = cJSON_Parse(str);
    Dict *input_dict = cJSON_to_dict(input->child);
    if(dict_get_type(input_dict,"content") == T_POINT_CHAR){
	cJSON *ctemp = cJSON_Parse((char*)dict_get_val(input_dict,"content"));
    	dict_remove_entry(input_dict,"content");
    	dict_put(input_dict,DYN_STR("content"),cJSON_to_dict(ctemp->child));
	cJSON_Delete(ctemp);
    }
    content = cJSON_GetObjectItem(input, "content");
    if(content->type == cJSON_String){
    	content = cJSON_Parse(content->valuestring);
    }
    socket_message_content *msg_c = (socket_message_content *) malloc(sizeof(socket_message_content));
    Dict* tmp = (Dict*)dict_get_val(input_dict, "content");
    if(dict_has_key(tmp, "actions")){
    	parse_actions_dict((Dict*)dict_get_val(tmp,"actions"),msg_c);
    }
    if (dict_has_key((Dict*)dict_get_val(input_dict,"content"),"meta")){
	msg_c->meta = (Dict*)dict_get_val(input_dict,"content");
    	dict_detatch_entry(input_dict,"content");
    }

    else{
    	msg_c->meta = NULL;
    }
    socket_message *msg = (socket_message *) malloc(sizeof(socket_message));
    msg->datetime = malloc(sizeof(struct tm));
    if(dict_get_type(input_dict,"datetime") != T_NULL){
    	parse_date((char*)dict_get_val(input_dict,"datetime"),msg->datetime);
    }
    else{
    	free(msg->datetime);
    	msg->datetime = NULL;
    }
    msg->action = parse_action((char*)dict_get_val(input_dict,"action"));
    msg->content = msg_c;
    msg->src = parse_pie_dict_val(dict_get_val(input_dict,"src"));
    if(dict_has_key(input_dict,"dest")){msg->dest = parse_pie_dict_val(dict_get_val(input_dict,"dest"));}
    msg->plugin_dest = DYN_STR((char*)dict_get_val(input_dict,"pluginDest"));
    cJSON_Delete(input);
    //dump_dict(input_dict);
    delete_dict_and_contents(input_dict);
    return msg;
}

plugin_message *json_to_plugin_message(cJSON *input){
    Dict *input_dict = cJSON_to_dict(input->child);
    if(!dict_has_key(input_dict,"src") || 
       !dict_has_key(input_dict,"pluginDest") || 
       !dict_has_key(input_dict,"content")){
        // Invalid Message
        return NULL;
    }
    if(dict_get_type(input_dict,"content") == T_POINT_CHAR){
        cJSON *ctemp = cJSON_Parse((char*)dict_get_val(input_dict,"content"));
        dict_remove_entry(input_dict,"content");
        dict_put(input_dict,DYN_STR("content"),cJSON_to_dict(ctemp->child));
        cJSON_Delete(ctemp);
    }
    plugin_message *msg = malloc(sizeof(plugin_message));
    msg->src = DYN_STR((char*)dict_get_val(input_dict,"src"));
    if(dict_has_key(input_dict,"dest")){msg->dest = DYN_STR((char*)dict_get_val(input_dict,"dest"));}
    else{msg->dest = NULL;}
    msg->plugin_dest = DYN_STR((char*)dict_get_val(input_dict,"pluginDest"));
    msg->content = (Dict*)dict_get_val(input_dict,"content");
    dict_detatch_entry(input_dict,"content");
    cJSON_Delete(input);
    delete_dict_and_contents(input_dict);
    return msg;
}

wrapped_message *json_to_message(char *str){
    cJSON *input = cJSON_Parse(str);
    /*cJSON *content = cJSON_GetObjectItem(input,"content");
    if(content && content->type == cJSON_String){
        content->child = cJSON_Parse(content->valuestring)->child;
        content->valuestring = NULL;
        content->type = cJSON_Object;
    }*/
    wrapped_message *ret = malloc(sizeof(wrapped_message));
    ret->is_socket_msg = socket_message_pred(input);
    if(ret->is_socket_msg){
        ret->sm = json_to_socket_message(input);
    }
    else{
        ret->pm = json_to_plugin_message(input);
    }
    return ret;
}



char *message_to_json(socket_message *msg) {

    cJSON *root, *content;
    root = cJSON_CreateObject();
    char datetime[27];
    strftime(datetime, sizeof(datetime), "%FT%T%z", msg->datetime);
    cJSON_AddStringToObject(root, "datetime", datetime);
    cJSON_AddStringToObject(root, "action", action_string(msg->action));
    cJSON_AddStringToObject(root, "src", pie_to_json(msg->src));
    if(msg->dest){cJSON_AddStringToObject(root, "dest", pie_to_json(msg->dest));}
    if(msg->plugin_dest){
    	cJSON_AddStringToObject(root, "pluginDest", msg->plugin_dest);
    }
    cJSON_AddItemToObject(root, "content", content = cJSON_CreateObject());
    cJSON_AddItemToObject(content, "actions", actions_to_json(msg->content->actions, msg->content->num_actions));
    if(msg->content->meta){
    	cJSON_AddItemToObject(content, "meta", dict_to_cJSON(msg->content->meta));
    }
    char *ret = cJSON_Print(root);
    cJSON_Delete(root);
    return ret;
}

// Warning: Deletes dictionary
char *dict_to_raw_json(Dict *dct) {
    cJSON *parsed = dict_to_cJSON(dct);
    delete_dict_and_contents(dct);
    char* ret = cJSON_PrintUnformatted(parsed);
    cJSON_Delete(parsed);
    return ret;
}

inline static void del_meta(Dict* tdel){
	if(tdel){
		if(tdel->type == T_ARR || tdel->type == T_DICT){
			delete_dict_and_contents(tdel->value);
		}else{
			free(tdel->value);
		}
	}
	free(tdel);
}
inline static void del_action_data(action_data* action){
	if(action->type == ADT_SLIDE){
		free(action->slide_data->location);
		free(action->slide_data);
	}else{
		printf("Uknown action type...\n");
	}
	free(action);
}
inline static void del_actions(action_data** actions, int num){
	for(--num;num >= 0;num--){
		del_action_data(actions[num]);
	}
	free(actions);
}

inline static void del_content(socket_message_content* tdel){
	if(tdel){
		del_meta(tdel->meta);
		del_actions(tdel->actions, tdel->num_actions);
	}
	free(tdel);
}

inline static void del_pie_struct(pie* p){
	if(p){
		free(p->name);
	}
	free(p);
}
inline static void del_tm(struct tm* dt){
	free(dt);
}
void delete_socket_message(socket_message* m){
	del_tm(m->datetime);
	free(m->plugin_dest);
	del_content(m->content);
	del_pie_struct(m->src);
	del_pie_struct(m->dest);
	free(m);
}

void wrapped_message_cleanup(wrapped_message *msg){
    if(msg->is_socket_msg){
        if(msg->sm)
            delete_socket_message(msg->sm);
    }
    else{
        free(msg->pm->src);
        free(msg->pm->dest);
        free(msg->pm->plugin_dest);
        if(msg->pm->content)
            delete_dict_and_contents(msg->pm->content);
    }
    free(msg);
}
void dump_message_json_str(char* str){
	cJSON *input;
	input = cJSON_Parse(str);
	if(cJSON_GetObjectItem(input,"content")){
		if(cJSON_GetObjectItem(input,"content")->type == cJSON_String){
			cJSON_ReplaceItemInObject(input,"content",cJSON_Parse(cJSON_GetObjectItem(input,"content")->valuestring));
		}
	}
	printf("%s",cJSON_Print(input));
	cJSON_Delete(input);
	return;
}

/*
void delete_socket_message(socket_message *m){
	if (m->content){
        if(!m->content->meta){
            printf("Meta is not null...\n");
        }
        else{
        	printf("Deleting dict and contents...\n");
        	delete_dict_and_contents(m->content->meta);
        }
        printf("Doing actions stuff....\n");
		if(m->content->actions){
			int i = 0;
			for(;i < m->content->num_actions;i++){
                action_data* data = m->content->actions[i];
                if(data->type == ADT_SLIDE){
                    free(data->slide_data);
                }
				free(data);
				m->content->actions[i] = NULL;
			}
		}
        free(m->content->actions);
        m->content->meta = NULL;
        m->content->actions = NULL;
	}
    free(m->content);
    free(m->datetime);
	if(m->src){
		free(m->src->name);
		free(m->src);
	}
	if(m->dest){
		free(m->dest->name);
		free(m->dest);
	}
	m->content = NULL;
	m->datetime = NULL;
	m->src = NULL;
	m->dest = NULL;
	free(m);
}
*/

/*
//Testing function...compile with gcc parseJSON.c cJSON.c dict.c -lm -lrt
//char json_string[] = "{\"datetime\" : \"2014-11-30T22:04:15+0000\",\"action\" : \"add-slide\",\"pies\" : [{ \"name\" : \"shepard\" },{ \"name\" : \"blueberry\" }],\"content\"  : {\"ID\" : 14,\"Permalink\" : \"http://dds-wp...\", \"meta\" : {\"key1\" : [ \"value\" ],\"key2\" : [\"value1\",\"value2\",3,{\"meta can be weird\" : \"remember that\"}]}}}";
//char json_string[] = "{\"src\" : \"WPHandler\", \"dest\" : \"keylime\", \"datetime\" : \"2014-11-30T22:04:15+0000\", \"content\" : {\"actions\" : [{\"ID\" : 226,\"type\" : \"slide\",\"location\" : \"http:\\/\\/www.ccs.neu.edu\\/systems\\/labstats\\/212.html\",\"duration\" : 1},{\"ID\" : 194,\"type\" : \"slide\",\"location\" : \"http:\\/\\/10.0.0.202\\/weather2\\/\",\"duration\" : 15}]}, \"pluginDest\" : \"slideShow\", \"action\" : \"load-slides\"}";
char json_string[] = "{\"src\" : \"WPHandler\", \"dest\" : \"keylime\", \"datetime\" : \"2014-11-30T22:04:15+0000\", \"content\" : {\"actions\" : [{\"ID\" : 226,\"type\" : \"slide\",\"location\" : \"http:\\/\\/www.ccs.neu.edu\\/systems\\/labstats\\/212.html\",\"duration\" : 1},{\"ID\" : 194,\"type\" : \"slide\",\"location\" : \"http:\\/\\/10.0.0.202\\/weather2\\/\",\"duration\" : 15}], \"meta\" : {\"key1\" : [ \"value\" ],\"key2\" : [\"value1\",\"value2\",3,{\"meta can be weird\" : \"remember that\"}]}}, \"pluginDest\" : \"slideShow\", \"action\" : \"load-slides\"}";
//char json_string[] = "{\"src\": \"WPHandler\", \"dest\": \"keylime\", \"datetime\": \"2014-11-30T22:04:15+0000\", \"content\": {\"actions\":[{\"type\":\"slide\",\"ID\": 12, \"location\":\"https:\\/\\/twitter.com\\/swiftonsecurity\",\"duration\":20},{\"type\":\"slide\",\"ID\": 27, \"location\":\"http:\\/\\/dds-wp.ccs.neu.edu\\/?slide=t-rex-trying&pie_name=keylime\",\"duration\":5},{\"type\":\"slide\",\"ID\": 55, \"location\":\"http:\\/\\/www.ccs.neu.edu\\/systems\\/labstats\\/212.html\",\"duration\":1},{\"type\":\"slide\",\"ID\": 94, \"location\":\"http:\\/\\/radar.weather.gov\\/ridge\\/Conus\\/Loop\\/NatLoop.gif\",\"duration\":20}]}, \"pluginDest\": \"slideShow\", \"action\": \"load-slides\"}";

//char json_string[] = "{\"src\": \"WPHandler\", \"dest\": \"keylime\", \"datetime\": \"2014-12-30T21:53:45+0000\", \"content\": {\"actions\":[{\"ID\":5,\"type\":\"slide\",\"location\":\"http:\\/\\/192.168.11.132\\/?slide=test1&pie_name=keylime\",\"duration\":1}]}, \"pluginDest\": \"slideShow\", \"action\": \"load-slides\"}";
int main() {
    socket_message *sm = json_to_message(json_string);
    #define DT_BUF_SIZE 27
    char dt[ DT_BUF_SIZE ];
    strftime(dt, sizeof(dt), "%Y-%m-%dT%H:%M:%S%z", sm->datetime);
    printf("sm:\ndatetime: %s\naction: %s\nsrc: %s\ndest: %s\n",
            dt, action_string(sm->action), sm->src->name, sm->dest->name);
    if(sm->content->meta){
    	printf("meta:\n\t\n...\n");
    	dump_dict(sm->content->meta);
    }
    printf("back again:\n%s\n", message_to_json(sm));
}*/
/*
#ifdef ___TEST_SUITES___

/*
 * TEST SUITES
 */
/*


#endif
*/
