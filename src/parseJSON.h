#ifndef PARSE_JSON_H
#define PARSE_JSON_H

#ifndef cJSON__h
#include "cJSON.h"
#endif

#ifndef DICT_H
#include "dict.h"
#endif

typedef enum {
    ADD_SLIDE, DELETE_SLIDE, EDIT_SLIDE, TERMINATE
} slide_action;

typedef enum {
    T_INT, T_CHAR, T_POINT_INT, T_POINT_CHAR, T_POINT_VOID, T_ARR, T_DICT
} META_TYPE;

typedef struct pie_struct {
    char *name;
} pie;

typedef struct m_meta_struct{
    void *value;
    META_TYPE type;
} socket_meta;

typedef struct m_content_struct {
    int id;
    char *permalink;
    Dict *meta;
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