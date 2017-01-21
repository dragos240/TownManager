typedef struct{
	char* townname;
	unsigned char mediatype_val;
} conftok_t;

int conf_parse(const char* data, conftok_t* token);
int conf_gen(char** data, conftok_t* token); //data must be a pointer!
