#include "dds_slides.h"


//MEMCHECK PASSED (I ALSO FOUND SOME BUGS DOING THIS...
slide* make_slide(char* loc, int dur, int id){
	slide* s = (slide*)malloc(sizeof(slide));
	if(strlen(loc) > URL_LEN - 1){
		printf("WARNING make_slide got a url > %d char's, truncating...\n", URL_LEN - 1);
		strncpy(s->location, loc, URL_LEN);
	}else{
		strcpy(s->location, loc);
	}
	if(dur < 0){
		printf("slide dur is %d, fixing at 0\n", dur);
		dur = 0;
	}
	s->dur = dur;
	s->id = id;

	return s;
}
void delete_slide(slide* s){
	free(s);
}

slide_list make_list_s(slide* s){
	list_node* node;
	slide_list list;
	node = (list_node*)malloc(sizeof(list_node));
	node->data = s;
	node->next = node;
	list = (list_iter*)malloc(sizeof(list_iter));
	list->list_start = node;
	list->cur = node;
	return list;
}
slide_list make_list(char* loc, int dur, int id){
	return make_list_s(make_slide(loc,dur,id));
}
static void delete_last(slide_list sl){	
	list_node* index = sl->list_start;
	while(index->next != sl->list_start){
		index = index->next;
	}
	list_node* tmp = sl->list_start;
	while(tmp->next != index){
		tmp = tmp->next;
	}
	tmp->next = sl->list_start;
	if(sl->cur == index){
		sl->cur = sl->list_start;
	}
	delete_slide(index->data);
	free(index);
}
void delete_list(slide_list sl){
	while(sl->list_start->next != sl->list_start){
		delete_last(sl);
	}
	delete_slide(sl->list_start->data);
	free(sl->list_start);
	free(sl);
}

int advance_list_index(slide_list l){
	if(l->cur->next == l->list_start){
		l->cur = l->list_start;
		return 0;

	}else{
		l->cur = l->cur->next;
		return 1;
	}
}
int delete_and_advance(slide_list l){
	delete_slide_with_id(l, l->cur->data->id);
/*	list_node* before_cur = l->list_start;
	while(before_cur->next != l->cur){before_cur = before_cur->next;}
	if(l->cur == l->list_start){
		l->list_start = l->cur->next;
	}
	before_cur->next = l->cur->next;
	list_node* cur_hold = l->cur;
	l->cur = l->cur->next;
	delete_slide(cur_hold->data);
	free(cur_hold);
*/	return 1;
}
slide* get_current_slide(slide_list s){
	return s->cur->data;
}
int add_slide(slide_list list, slide* slide){
	list_node* node = (list_node*)malloc(sizeof(list_node));
	node->data = slide;
	node->next = list->list_start;
	list_node* tmp = list->list_start;
	while(tmp->next != list->list_start){tmp = tmp->next;}
	tmp->next = node;
	return 1;
}

void delete_slide_with_id(slide_list sl, int id){
	list_node* target = sl->list_start;
	while(target->data->id != id){
		if(target->next == sl->list_start){
			//not in list...
			return;
		}
		target = target->next;
	}

	list_node* before_target = sl->list_start;
	while(before_target->next != target){before_target = before_target->next;}
	if(target == sl->list_start){
		sl->list_start = target->next;
	}
	if(target == sl->cur){
		sl->cur = target->next;
	}
	before_target->next = target->next;
	delete_slide(target->data);
	free(target);
}
int set_slide_with_id(slide_list sl, char* loc, int dur, int id){
	return set_slide_with_id_s(sl, make_slide(loc, dur, id));
}
int set_slide_with_id_s(slide_list sl, slide* s){
	list_node* node = sl->list_start;
	while(node->data->id != s->id){
		if(node->next == sl->list_start){
			//Not in list
			return 0;
		}
		node = node->next;
	}
	delete_slide(node->data);
	node->data = s;
	return 1;
}
