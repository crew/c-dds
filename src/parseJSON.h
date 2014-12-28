#ifndef PARSE_JSON_H
#define PARSE_JSON_H

#ifndef cJSON__h
#include "cJSON.h"
#endif

#ifndef DICT_H
#include "dict.h"
#endif

#include <stdarg.h>

#define SERVER_PI (pie){"DDS_SERVER"}
#define REG_ACTION_INFO(type,strct) typedef struct strct type##_action_info

typedef enum {
    ADD_SLIDE, DELETE_SLIDE, EDIT_SLIDE, LOAD_SLIDES, TERMINATE
} SLIDE_ACTION;

typedef enum {
    T_INT, T_CHAR, T_POINT_INT, T_POINT_CHAR, T_POINT_VOID, T_ARR, T_DICT
} META_TYPE;

typedef enum {ADT_SLIDE} ACTION_DATA_TYPE;

REG_ACTION_INFO(slide,{int id; char *location; int duration;});

typedef struct action_data_struct {
	ACTION_DATA_TYPE type;
	union{ // In case we would like additional actions later,
		   //  this union encapsulates everything slide-specific
		 slide_action_info *slide_data;
	};
} action_data;

typedef struct pie_struct {
    char *name;
} pie;

typedef struct m_meta_struct{
    void *value;
    META_TYPE type;
} socket_meta;

typedef struct m_content_struct {
    Dict *meta;
    int num_actions;
    action_data **actions;
} socket_message_content;

typedef struct m_root_struct {
    struct tm *datetime;
    SLIDE_ACTION action;
    char *plugin_dest;
    socket_message_content *content;
    pie *src;
    pie *dest;
} socket_message;

extern socket_message *json_to_message(char *str);
extern char *message_to_json(socket_message *msg);

#endif
