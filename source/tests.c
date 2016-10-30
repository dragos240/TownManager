#include <stdio.h>

#include <3ds.h>
#include <sf2d.h>

#include "common.h"
#include "menu.h"
#include "ui.h"
#include "gfx.h"
#include "kb.h"
#include "launcher.h"
#include "backup.h"
#include "tests.h"


void run_tests(){
	//text_test();
	//simple_gfx_test();
	//gfx_test();
	menu_test();
	kb_test();
	launcher_test();
}


void text_test(){
	gfxInitDefault();

	consoleInit(GFX_TOP, NULL);

	printf("Hello world!\n");

	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();

	while(aptMainLoop()){
		hidScanInput();

		if(hidKeysDown() & KEY_A)
			break;
	}

	gfxExit();
}

void simple_gfx_test(){
	sf2d_init();

	while(aptMainLoop()){
		hidScanInput();

		if(hidKeysDown() & KEY_A)
			break;
		
		sf2d_start_frame(GFX_TOP, GFX_LEFT);
			sf2d_draw_rectangle(0, 0, 200, 200, RGBA8(0xFF, 0xFF, 0xFF, 0xFF));
		sf2d_end_frame();
		sf2d_swapbuffers();
	}

	sf2d_fini();
}

void gfx_test(){
	gfx_init();

	while(aptMainLoop()){
		hidScanInput();

		if(hidKeysDown() & KEY_A)
			break;
	}

	gfx_fini();
}

void menu_test(){
	int menuindex = 0;
	int menucount = 2;

	char* headerstr = "TownManager";

	char* menu_entries[] = {
		"Start game",
		"Exit"
	};

	gfx_init();

	while(aptMainLoop() && menuindex != -1){
		display_menu(menu_entries, menucount, &menuindex, headerstr);

		if(menuindex == -1)
			break;

		switch(menuindex){
			case 0:
				//blah
				break;
			case 1:
				menuindex = -1;
				break;
		}
	}

	gfx_fini();
}

void kb_test(){
}

void launcher_test(){
}
