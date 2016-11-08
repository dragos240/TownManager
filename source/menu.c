#include <stdlib.h>
#include <string.h>

#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>

#include "gfx.h"
#include "ui.h"
#include "menu.h"

static int fontheight = 11;
//static int fontwidth = 7;

void display_menu(char **menu_entries, int total_entries, int *menupos, char *headerstr){
	int i, j;
	u32 kDown = 0;
	int rows = 21; //27 changed to 21 due to font size changing
	int startpos = 0;

	while(aptMainLoop()){
		hidScanInput();
		
		kDown = hidKeysDown();
	
		sf2d_start_frame(GFX_TOP, GFX_LEFT);
			ui_frame(headerstr);

			j = 0;
			//if more total_entries than max rows
			if(total_entries > rows){
				for(i = startpos; i < startpos+rows-2; i++){
					if(i == *menupos){
						sf2d_draw_texture(arrow, fontheight, (j+2)*fontheight+1);
					}
					sftd_draw_textf(font, fontheight*3, (j+2)*fontheight, COLOR_WHITE, fontheight, "%s", menu_entries[i]);
					current_pos.x = fontheight*3 + strlen(menu_entries[i]);
					current_pos.y = (j+3)*fontheight;
					j++;
				}
			}
			//if more max rows than total entries
			else{
				for(i = 0; i < total_entries; i++){
					if(i == *menupos){
						sf2d_draw_texture(arrow, fontheight, (i+2)*fontheight+1);
					}
					sftd_draw_textf(font, fontheight*3, (i+2)*fontheight, COLOR_WHITE, fontheight, "%s", menu_entries[i]);
					current_pos.x = fontheight*3 + strlen(menu_entries[i]);
					current_pos.y = (i+3)*fontheight;
				}
			}
		sf2d_end_frame();
		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		sf2d_end_frame();
		sf2d_swapbuffers();

		if(kDown & (KEY_UP | KEY_LEFT)){
			(*menupos)--;
			if(*menupos < 0){
				startpos = total_entries-rows+2;
				*menupos = total_entries-1;
			}
			else if(*menupos < startpos){
				startpos--;
			}
		}
		else if(kDown & (KEY_DOWN | KEY_RIGHT)){
			(*menupos)++;
			if(*menupos == total_entries){
				startpos = 0;
				*menupos = 0;
			}
			else if((*menupos-startpos) > rows-3)
				startpos++;
		}
		else if(kDown & KEY_A)
			return;
		else if(kDown & KEY_B){
			*menupos = -1;
			return;
		}
	}
}
