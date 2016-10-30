#ifndef TM_COMMON_H
#define TM_COMMON_H

#include <3ds.h>

#define GAMEID 0x0004000000086300
#define SECOND_IN_NS (1000*1000*1000)
#define SIZE_GARDEN 522752
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

extern bool is3dsx;
extern int debug;

void setIs3dsx();

char* u16str2str(u16* wstr);

#endif
