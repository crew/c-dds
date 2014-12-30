#ifndef DDS_SLIDES_H
#define DDS_SLIDES_H


#define URL_LEN 1024


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
typedef struct _slide{
	//NOTE: limits urls to 1024 char's
	char location[URL_LEN];
	int dur;
	int id;
}slide;

typedef struct _s_list_node{
	slide* data;
	struct _s_list_node* next;
}list_node;
typedef struct _slide_list_iterator{
	list_node* list_start;
	list_node* cur;
}list_iter;

typedef list_iter* slide_list;

slide* make_slide(char* loc, int dur, int id);
void delete_slide(slide* s);

slide_list make_list_s(slide* s);
slide_list make_list(char* loc, int dur, int id);
void delete_list(slide_list sl);
int advance_list_index(slide_list);
int delete_and_advance(slide_list);
slide* get_current_slide(slide_list);

int add_slide(slide_list, slide*);
void delete_slide_with_id(slide_list sl, int id);
int set_slide_with_id(slide_list sl, char* loc, int dur, int id);
int set_slide_with_id_s(slide_list sl, slide* s);
#endif
