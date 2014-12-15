typedef struct _dict{
	char* key;
	void* value;
	struct _dict* next;
}Dict;

Dict* make_dict(void);
int dict_size(Dict*);
void delete_dict(Dict*);
int dict_has_key(Dict* dict, char* key);
void* dict_get_val(Dict* dict, char* key);
void* dict_put(Dict* dict, char* key, void* val);
int dict_remove_entry(Dict* dict, char* key); 
