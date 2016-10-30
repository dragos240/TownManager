#ifndef TM_GFX_H
#define TM_GFX_H

#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>

extern sf2d_texture* arrow;
extern sftd_font* font;
extern sftd_font* font_bold;

typedef struct{
	int x;
	int y;
} pos_t;

extern pos_t current_pos;

extern const int COLOR_WHITE;
extern const int COLOR_BLACK;

void gfx_init();
void gfx_fini();

void gfx_waitbutton();
int gfx_prompt(char* message, char* keymsg);
int gfx_prompt3(char* message, char* keymsg);
void gfx_error(Result ret, char* file, int line);
void gfx_displaymessage(char* msg, ...);
void gfx_waitmessage(char* msg, ...);

#endif
