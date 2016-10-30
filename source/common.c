#include <stdlib.h>

#include <3ds.h>

bool is3dsx;

void setIs3dsx(){
	u64 id;
	APT_GetProgramID(&id);

	if(id == 0x0004000001198300){
		is3dsx = false;
		return;
	}

	is3dsx = true;
}

char* u16str2str(u16* wstr){
	int len = 0;
	char* str;

	str = malloc(1);
	len++;
	while(wstr[len-1] < 128){
		str[len-1] = (char)(wstr[len-1]);
		len++;
		str = realloc(str, len);
	}

	return str;
}
