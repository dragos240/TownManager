#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <3ds.h>

#include "common.h"
#include "gfx.h"
#include "menu.h"
#include "fs.h"
#include "backup.h"
#include "kb.h"
#include "launcher.h"
#include "town.h"

void populate_menu_entries(char*** menu_entries, int* menucount){
	//Result ret;
	dir_t dirs;
	int i;

	dirs = get_dirs("/TownManager/Saves/");

	*menucount = dirs.numdirs+1;
	*menu_entries = malloc((dirs.numdirs+1)*sizeof(char*));
	for(i = 0; i < dirs.numdirs; i++){
		(*menu_entries)[i] = dirs.dirs[i];
	}
	(*menu_entries)[dirs.numdirs] = "New";
}

void create_town(){
	Result ret;
	Handle handle;
	u32 written;
	char* townname;
	char* savesdir = "/TownManager/Saves/";
	char* towndir;
	char* filepath;
	file_t files;
	int i;

	townname = get_string("Enter a town name");
	if(strcmp(townname, "") == 0)
		return;

	//Create townname directory
	towndir = calloc(strlen(savesdir)+strlen(townname)+1, 1);
	sprintf(towndir, "%s%s", savesdir, townname);
	ret = FSUSER_CreateDirectory(sdmc_arch, fsMakePath(PATH_ASCII, towndir), 0);
	free(towndir);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	//Write townname as the sole contents of the /TownManager/tm.conf file
	ret = FSUSER_OpenFile(&handle, sdmc_arch, fsMakePath(PATH_ASCII, "/TownManager/tm.conf"), FS_OPEN_WRITE, 0);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	ret = FSFILE_Write(handle, &written, 0, townname, strlen(townname)+1, FS_WRITE_FLUSH | FS_WRITE_UPDATE_TIME);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	ret = FSFILE_SetSize(handle, (u64)(strlen(townname)+1));
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	ret = FSFILE_Close(handle);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	//Erase game data
	files = get_files(game_arch, "/");
	for(i = 0; i < files.numfiles; i++){
		filepath = calloc(1+strlen(files.files[i])+1, 1);
		sprintf(filepath, "/%s", files.files[i]);
		ret = FSUSER_DeleteFile(game_arch, fsMakePath(PATH_ASCII, filepath));
		if(ret){
			gfx_error(ret, __FILENAME__, __LINE__);
			return;
		}
		free(filepath);
	}
	ret = FSUSER_ControlArchive(game_arch, ARCHIVE_ACTION_COMMIT_SAVE_DATA, NULL, 0, NULL, 0);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	if(is3dsx){
		gfx_displaymessage("Launching game... (this may take ~10 seconds)");
	}
	launch_game();
	svcSleepThread(2*SECOND_IN_NS); //so that the last message gets displayed and stays there as the game launches
}

void init_save_folder(){
	Result ret;
	int error;
	file_t files;
	char* buffer;
	char* filepath;
	u64 size;
	int i;

	gfx_waitmessage("No save folders detected! Importing current saves as 'Main'.");
	ret = FSUSER_CreateDirectory(sdmc_arch, fsMakePath(PATH_ASCII, "/TownManager/Saves/Main"), 0);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	files = get_files(game_arch, "/");
	for(i = 0; i < files.numfiles; i++){
		gfx_displaymessage("Backing up %s...", files.files[i]);
		filepath = calloc((strlen(files.files[i])+2), 1);
		sprintf(filepath, "/%s", files.files[i]);
		buffer = file_to_buffer(game_arch, "/", files.files[i]);
		if((strcmp(buffer, "")) == 0)
			break;
		size = filesize_to_u64(game_arch, filepath);
		if((int)size == -1)
			break;
		error = buffer_to_file(sdmc_arch, buffer, size, "/TownManager/Saves/Main/", files.files[i]);
		if(error == -1)
			break;
		free(filepath);
	}
	gfx_waitmessage("Done!");
}

void load_tm_config(char** current){
	Result ret;
	Handle handle;
	u64 size;
	u32 read;
	char* buf = NULL;
	int i;
	int eol = 0;
	char* buf2;

	FSUSER_CreateDirectory(sdmc_arch, fsMakePath(PATH_ASCII, "/TownManager"), 0);
	ret = FSUSER_OpenFile(&handle, sdmc_arch, fsMakePath(PATH_ASCII, "/TownManager/tm.conf"), FS_OPEN_READ | FS_OPEN_CREATE, 0);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	ret = FSFILE_GetSize(handle, &size);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	if(size < 1){
		*current = "Main";
		return;
	}
	buf = malloc(size);
	ret = FSFILE_Read(handle, &read, 0, buf, (u32)size);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	if(read != (u32)size){
		gfx_waitmessage("Bytes read does not match size! (load_tm_config())");
		return;
	}

	for(i = 0; i < (int)read; i++){
		if(buf[i] == '\n'){
			break;
		}
		eol++;
	}
	buf2 = calloc(eol+1, 1);
	for(i = 0; i < eol; i++){
		buf2[i] = buf[i];
	}
	*current = buf2;
}

void town_opts(char* dirname){
	int menuindex = 0;
	int menucount = 4;
	char* current_town;
	int ret;

	char headerstr[] = "Town options";

	char* menu_entries[] = {
		"Launch town",
		"Clone town",
		"Rename town",
		"Delete town"
	};

	load_tm_config(&current_town);

	while(aptMainLoop()){
		display_menu(menu_entries, menucount, &menuindex, headerstr);

		if(menuindex == -1)
			break;

		switch(menuindex){
			case 0:
				//code to load game files and inject them into game
				ret = prepare_to_launch(dirname);
				if(ret == -1){
					gfx_waitmessage("Game failed to launch :(");
					break;
				}
				if(is3dsx){
					gfx_displaymessage("Launching game... (this may take ~10 seconds)");
				}
				launch_game();
				svcSleepThread(2*SECOND_IN_NS); //so that the last message gets displayed and stays there as the game launches
				break;
			case 1:
				clone_town(dirname);
				return;
			case 2:
				rename_town(dirname);
				return;
			case 3:
				delete_town(dirname);
				return;
		}
	}
}

void clone_town(char* dirname){
	char* savespath = "/TownManager/Saves/";
	char* newtown;
	char newpath[strlen(savespath)+64+1+1]; //2 = '/' + '\0'
	char oldpath[strlen(savespath)+strlen(dirname)+1+1]; //2 = '/' + '\0'
	char* oldfilepath;
	char* buf;
	file_t files;
	u64 size;
	int i;

	newtown = get_string("Enter a name for the cloned town");
	if(strcmp(newtown, "") == 0){
		gfx_waitmessage("Nothing entered. Town not cloned");
		return;
	}
	sprintf(newpath, "%s%s/", savespath, newtown);
	FSUSER_CreateDirectory(sdmc_arch, fsMakePath(PATH_ASCII, newpath), 0);

	memset(oldpath, '\0', sizeof(oldpath));
	sprintf(oldpath, "%s%s/", savespath, dirname);
	files = get_files(sdmc_arch, oldpath);

	for(i = 0; i < files.numfiles; i++){
		oldfilepath = calloc(strlen(oldpath)+strlen(files.files[i])+1, 1);
		sprintf(oldfilepath, "%s%s", oldpath, files.files[i]);
		buf = file_to_buffer(sdmc_arch, oldpath, files.files[i]);
		size = filesize_to_u64(sdmc_arch, oldfilepath);
		gfx_displaymessage("Copying %s to %s...", files.files[i], newtown);
		buffer_to_file(sdmc_arch, buf, size, newpath, files.files[i]);
		free(oldfilepath);
		free(buf);
	}
	gfx_waitmessage("Done cloning town!");
}

void rename_town(char* dirname){
	Result ret;
	char* savespath = "/TownManager/Saves/";
	char* newname;
	char oldpath[strlen(savespath)+strlen(dirname)+1];
	char newpath[strlen(savespath)+64+1];

	newname = get_string("Enter the new name of the town");
	memset(oldpath, '\0', sizeof(oldpath));
	sprintf(oldpath, "%s%s", savespath, dirname);
	memset(newpath, '\0', sizeof(newpath));
	sprintf(newpath, "%s%s", savespath, newname);
	gfx_displaymessage("Renaming town...");
	if((ret = FSUSER_RenameDirectory(sdmc_arch, fsMakePath(PATH_ASCII, oldpath), sdmc_arch, fsMakePath(PATH_ASCII, newpath)))){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	gfx_waitmessage("Done renaming town!");
}

void delete_town(char* dirname){
	Result ret;
	Handle handle;
	int error = 0;
	u32 read;
	u64 size;
	char* savespath = "/TownManager/Saves/";
	char* dirpath;
	char* lasttown;
	
	//check if town in tm.conf is the same as the selected town
	ret = FSUSER_OpenFile(&handle, sdmc_arch, fsMakePath(PATH_ASCII, "/TownManager/tm.conf"), FS_OPEN_READ, 0);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		error = 1;
		goto delete_town_cleanup;
	}
	ret = FSFILE_GetSize(handle, &size);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		error = 1;
		goto delete_town_cleanup;
	}
	lasttown = calloc(size+1, 1);
	ret = FSFILE_Read(handle, &read, 0, lasttown, (u32)size);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		error = 1;
		goto delete_town_cleanup;
	}
	FSFILE_Close(handle);
	//if same, delete tm.conf
	if(strcmp(dirname, "Main") == 0){
		gfx_waitmessage("'Main' cannot be deleted as it is the default save file.");
		error = 1;
		goto delete_town_cleanup;
	}
	if(strcmp(dirname, lasttown) == 0){
		gfx_displaymessage("Deleting tm.conf...");
		ret = FSUSER_DeleteFile(sdmc_arch, fsMakePath(PATH_ASCII, "/TownManager/tm.conf"));
		if(ret){
			gfx_error(ret, __FILENAME__, __LINE__);
			error = 1;
		}
	}
	free(lasttown);

	//Delete town folder
	dirpath = calloc(strlen(savespath)+strlen(dirname)+1, 1);
	sprintf(dirpath, "%s%s", savespath, dirname);
	gfx_displaymessage("Deleting town...");
	ret = FSUSER_DeleteDirectoryRecursively(sdmc_arch, fsMakePath(PATH_ASCII, dirpath));
	if(ret){
		gfx_waitmessage("dirpath: %s", dirpath);
		gfx_error(ret, __FILENAME__, __LINE__);
		error = 1;
		goto delete_town_cleanup;
	}
	free(dirpath);
delete_town_cleanup:
	if(!error)
		gfx_waitmessage("Successfully deleted save files");
	else
		gfx_waitmessage("Failed to delete save files");
}
