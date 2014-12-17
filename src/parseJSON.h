#ifndef PARSE_JSON_H
#define PARSE_JSON_H

#ifndef cJSON__h
#include "cJSON.h"
#endif

typedef enum {
    ADD_SLIDE, DELETE_SLIDE, EDIT_SLIDE, TERMINATE
} slide_action;

typedef struct pie_struct {
    char *name;
} pie;

typedef struct m_content_struct {
    int id;
    char *permalink;
    cJSON *meta;
} socket_message_content;

typedef struct m_root_struct {
    struct tm *datetime;
    slide_action action;
    socket_message_content *content;
    int pie_list_len;
    pie **pie_list;
} socket_message;

extern socket_message *json_to_message(char *str);
extern char *message_to_json(socket_message *msg);

#endif