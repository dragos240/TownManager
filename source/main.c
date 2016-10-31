#include <stdio.h>
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

int main(){
	Result ret;
	char* current_town;
	if(debug == 1){
		run_tests();
		return 0;
	}

	setIs3dsx();

	int menuindex = 0;
	int menucount;

	char headerstr[] = "TownManager";

	char** menu_entries = NULL;

	gfx_init();
	launcher_init();
	ret = fs_init();
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		goto main_cleanup;
	}

	load_tm_config(&current_town);
	if(strcmp(current_town, "") != 0){
		backup_to_prev_folder(current_town);
	}

	while(aptMainLoop() && menuindex != -1){
		populate_menu_entries(&menu_entries, &menucount);

		if(menucount == 1){
			init_save_folder();
		}
		else{
			display_menu(menu_entries, menucount, &menuindex, headerstr);
		}

		if(menuindex != menucount-1 && menuindex != -1){
			town_opts(menu_entries[menuindex]);
		}
		else if(menucount != 1 && menuindex != -1){
			create_town();
		}
	}
main_cleanup:
	fs_fini();
	launcher_fini();
	gfx_fini();

	return 0;
}
