#ifndef TM_COMMON_H
#define TM_COMMON_H

#include <3ds.h>

#define NA_GAMEID 0x0004000000086300
#define EU_GAMEID 0x0004000000086400
#define JP_GAMEID 0x0004000000086200
#define NA_WA_GAMEID 0x0004000000198E00
#define EU_WA_GAMEID 0x0004000000198F00
#define JP_WA_GAMEID 0x0004000000198D00
#define SECOND_IN_NS (1000*1000*1000)
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

extern bool is3dsx;
extern int debug;

void setIs3dsx();

char* u16str2str(u16* wstr);

#endif
