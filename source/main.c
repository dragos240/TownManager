#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include "common.h"
#include "menu.h"
#include "gfx.h"
#include "ui.h"
#include "fs.h"
#include "backup.h"
#include "town.h"
#include "tests.h"
#include "launcher.h"

int debug = 0;
const char* tmver = "1.2.1";

int main(){
	Result ret;
	char* current_town = NULL;
	if(debug == 1){
		run_tests();
		return 0;
	}

	setIs3dsx();

	int menuindex = 0;
	int menucount;

	char* headerstr;
	headerstr = calloc(strlen("TownManager - v")+10+1, 1); //give 10 chars for ver string
	sprintf(headerstr, "TownManager - v%s", tmver);

	char** menu_entries = NULL;

	gfx_init();
	if(is3dsx){
		if((ret = fs_init())){
			gfx_error(ret, __FILENAME__, __LINE__);
			goto main_cleanup;
		}
	}
	//open sdmc archive
	if((ret = open_sdmc_archive())){
		gfx_error(ret, __FILENAME__, __LINE__);
		goto main_cleanup;
	}
	//create sdmc:/TownManager if it does not already exist
	FSUSER_CreateDirectory(sdmc_arch, fsMakePath(PATH_ASCII, "/TownManager"), 0);
	//load configuration file
	if(load_config(&current_town) < 0){
		gfx_waitmessage("load_config failed");
		goto main_cleanup;
	}
	//get titleid (titleid varies depending on region)
	if(get_titleid() == -1){
		gfx_waitmessage("get_titleid failed");
		goto main_cleanup;
	}
	//initialize launcher
	launcher_init();
	//open game archive
	if((ret = open_game_archive())){
		gfx_error(ret, __FILENAME__, __LINE__);
		goto main_cleanup;
	}

	if(current_town != NULL){
		backup_to_prev_folder(current_town);
	}

	while(aptMainLoop() && menuindex != -1){
		populate_menu_entries(&menu_entries, &menucount);

		display_menu(menu_entries, menucount, &menuindex, headerstr);

		if(menuindex < menucount-2 && menuindex != -1){
			town_opts(menu_entries[menuindex]);
		}
		else if(menuindex == menucount-2){
			create_town();
		}
		else if(menuindex == menucount-1){
			change_mediatype();
		}
	}
main_cleanup:
	close_archives();
	if(is3dsx){
		fs_fini();
	}
	launcher_fini();
	gfx_fini();

	return 0;
}
