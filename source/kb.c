#include <stdlib.h>

#include <3ds.h>

#include "kb.h"

char* get_string(char* hint){
	SwkbdState key_state;
	char* input = calloc(64, 1);

	swkbdInit(&key_state, SWKBD_TYPE_QWERTY, 2, 64);
	swkbdSetHintText(&key_state, hint);

	swkbdInputText(&key_state, input, 64);

	return input;
}
