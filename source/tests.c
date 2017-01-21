#include <stdio.h>
#include <stdlib.h>

#include <3ds.h>
#include <sf2d.h>

#include "common.h"
#include "menu.h"
#include "ui.h"
#include "gfx.h"
#include "kb.h"
#include "launcher.h"
#include "backup.h"
#include "fs.h"
#include "conf.h"
#include "tests.h"


void run_tests(){
	conf_parse_test();
	//text_test();
	//simple_gfx_test();
	//gfx_test();
	//menu_test();
	//kb_test();
	//launcher_test();
}

void conf_parse_test(){
	Handle handle;
	u32 written;
	Result ret;
	char* orig_str1;
	char* reconst_str1;
	conftok_t token = {NULL, 0};


	gfx_init();
	open_sdmc_archive();

	const char* townname = "Tokyo";
	const int mediatype_val = 1;
	//str format: (length of townname (2 bytes))townname(mediatype_val (1 byte))
	orig_str1 = calloc(2+strlen(townname)+1+1, 1); //refer to above, last 1 = '\0'
	sprintf(orig_str1, "%02d%s%c", strlen(townname), townname, mediatype_val);
	conf_parse(orig_str1, &token);
	gfx_waitmessage("townname:");
	gfx_waitmessage((char*)token.townname);
	gfx_waitmessage("mediatype:");
	gfx_waitmessage("%d", token.mediatype_val);
	//goto conf_parse_test_cleanup; //temporary
	conf_gen(&reconst_str1, &token);

	gfx_waitmessage("Reconstructed str:%s", reconst_str1);

	FSUSER_CreateDirectory(sdmc_arch, fsMakePath(PATH_ASCII, "/TownManager"), 0);
	if((ret = FSUSER_OpenFile(&handle, sdmc_arch, fsMakePath(PATH_ASCII, "/TownManager/test.txt"), FS_OPEN_WRITE | FS_OPEN_CREATE, 0))){
		gfx_error(ret, __FILENAME__, __LINE__);
		goto conf_parse_test_cleanup;
	}
	if((ret = FSFILE_Write(handle, &written, 0, reconst_str1, strlen(reconst_str1), FS_WRITE_FLUSH | FS_WRITE_UPDATE_TIME))){
		gfx_error(ret, __FILENAME__, __LINE__);
		goto conf_parse_test_cleanup;
	}
	if((ret = FSFILE_SetSize(handle, strlen(reconst_str1)))){
		gfx_error(ret, __FILENAME__, __LINE__);
	}
	if(written != strlen(reconst_str1)){
		gfx_waitmessage("Error! size of reconst_str1 is %u and written in %u", strlen(reconst_str1), written);
	}
conf_parse_test_cleanup:
	if((ret = FSFILE_Close(handle))){
		gfx_error(ret, __FILENAME__, __LINE__);
	}

	gfx_fini();
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
