#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <syslog.h>
#include "cJSON.h"
#include "parseJSON.h"


//char json_string[] = "{\"datetime\" : \"2014-11-30T22:04:15+0000\",\"action\" : \"add-slide\",\"pies\" : [{ \"name\" : \"shepard\" },{ \"name\" : \"blueberry\" }],\"content\"  : {\"ID\" : 14,\"Permalink\" : \"http://dds-wp...\", \"meta\" : {\"key1\" : [ \"value\" ],\"key2\" : [\"value1\",\"value2\",3,{\"meta can be weird\" : \"remember that\"}]}}}";

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

const char *parse_date (const char *input, struct tm *tm)
{
    const char *cp;

    /* First clear the result structure.  */
    memset (tm, '\0', sizeof (*tm));

    /* Try the ISO format first.  */
    cp = strptime (input, "%FT%T%z", tm);
    if (cp == NULL)
    {
        /* Does not match.  Try the US form.  */
        cp = strptime (input, "%D", tm);
    }

    return cp;
}

socket_message *json_to_message(char *str) {
    cJSON *input, *content;
    input = cJSON_Parse(str);
    content = cJSON_GetObjectItem(input, "content");
    socket_message_content *msg_c = (socket_message_content *) malloc(sizeof(socket_message_content));
    msg_c->id = cJSON_GetObjectItem(content, "ID")->valueint;
    msg_c->permalink = cJSON_GetObjectItem(content, "Permalink")->valuestring;
    msg_c->meta = cJSON_GetObjectItem(content, "meta");
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
    cJSON_AddItemToObject(content, "meta", msg->content->meta);
    return cJSON_Print(root);
}











