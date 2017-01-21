#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "common.h"
#include "gfx.h"
#include "conf.h"

//tokens must be array or pre-allocated pointer
int conf_parse(const char* data, conftok_t* token){
	int len;

	len = data[0];
	token->townname = calloc(len+1, 1); //1 = '\0'
	sprintf(token->townname, "%.*s", len, &data[1]);
	token->mediatype_val = data[1+len];

	return 0;
}

//data must not be a pointer to an array
int conf_gen(char** data, conftok_t* token){
	//str format: (length of townname (1 bytes))townname(mediatype_val (1 byte))
	*data = calloc(1+strlen(token->townname+1+1), 1);
	sprintf(*data, "%c%s%c", strlen(token->townname), token->townname, token->mediatype_val);

	return 0;
}
