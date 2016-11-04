#include <string.h>
#include <stdlib.h>

#include <3ds.h>

#include "common.h"
#include "gfx.h"
#include "menu.h"
#include "fs.h"

FS_MediaType mediatype;
u64 titleid;
u32 lowerid, upperid, uniqueid;

FS_Archive game_arch, sdmc_arch;

Result fs_init(){
	Handle fs_handle;
	Result ret;
	fsInit();
	ret = srvGetServiceHandleDirect(&fs_handle, "fs:USER");
	if(ret)
		return ret;
	ret = FSUSER_Initialize(fs_handle);
	if(ret)
		return ret;
	fsUseSession(fs_handle);

	return ret;
}

Result open_archives(){
	Result ret;
	u32* path;
	FS_Path null_binpath, game_binpath;

	//open sdmc card
	null_binpath = (FS_Path){PATH_EMPTY, 1, (u8*)""};
	ret = FSUSER_OpenArchive(&sdmc_arch, ARCHIVE_SDMC, null_binpath);
	if(ret)
		return ret;

	if(is3dsx){
		ret = FSUSER_OpenArchive(&game_arch, ARCHIVE_SAVEDATA, fsMakePath(PATH_EMPTY, ""));
		if(ret)
			return ret;
	}
	else{
		path = (u32[3]){mediatype, lowerid, upperid};
		game_binpath = (FS_Path){PATH_BINARY, 12, path};
		ret = FSUSER_OpenArchive(&game_arch, ARCHIVE_USER_SAVEDATA, game_binpath);
		if(ret)
			return ret;
	}

	return ret;
}

Result close_archives(){
	Result ret;
	
	ret = FSUSER_CloseArchive(game_arch);
	ret |= FSUSER_CloseArchive(sdmc_arch);

	return ret;
}

void fs_fini(){
	fsEndUseSession();
	fsExit();
}

dir_t get_dirs(char* path){
	Result ret;
	Handle handle;
	dir_t dirs;
	u32 read;
	FS_DirectoryEntry get_ent;

	ret = FSUSER_OpenDirectory(&handle, sdmc_arch, fsMakePath(PATH_ASCII, path));
	if(ret){
		dirs.numdirs = 0;
		gfx_error(ret, __FILENAME__, __LINE__);
	}
	dirs.numdirs = 0;
	dirs.dirs = NULL;
	do{
		FSDIR_Read(handle, &read, 1, &get_ent);
		if(read < 1)
			break;
		dirs.numdirs++;
		dirs.dirs = realloc(dirs.dirs, dirs.numdirs*sizeof(char*));
		dirs.dirs[dirs.numdirs-1] = u16str2str(get_ent.name);
	}
	while(read > 0);

	return dirs;
}

file_t get_files(FS_Archive arch, char* path){
	Result ret;
	Handle handle;
	file_t files;
	u32 read;
	FS_DirectoryEntry get_ent;

	ret = FSUSER_OpenDirectory(&handle, arch, fsMakePath(PATH_ASCII, path));
	if(ret){
		files.numfiles = 0;
		memset(files.files, '\0', sizeof(char*));
		gfx_error(ret, __FILENAME__, __LINE__);
	}
	files.numfiles = 0;
	files.files = NULL;
	do{
		FSDIR_Read(handle, &read, 1, &get_ent);
		if(read < 1)
			break;
		files.numfiles++;
		files.files = realloc(files.files, files.numfiles*sizeof(char*));
		files.files[files.numfiles-1] = u16str2str(get_ent.name);
	}
	while(read > 0);

	return files;
}

void get_mediatype(){
	int menuindex = 0;
	int menucount = 2;

	char headerstr[] = "Select the mediatype of your game.";

	char* menu_entries[] = {
		"eShop/CIA version",
		"Cartridge"
	};

	if(is3dsx){
		FSUSER_GetMediaType(&mediatype);
	}
	else{
		display_menu(menu_entries, menucount, &menuindex, headerstr);
		mediatype = menuindex+1;
	}
}

int get_titleid(){
	Result ret;
	u32 num_sdtitles;
	u64* sd_titles;
	int i;

	amInit();
	if(mediatype == MEDIATYPE_GAME_CARD){
		AM_GetTitleList(NULL, mediatype, 1, &titleid);
	}
	else{
		gfx_displaymessage("Getting titleid of your eShop copy of AC:NL (this might take a while)...");
		ret = AM_GetTitleCount(mediatype, &num_sdtitles);
		if(ret){
			gfx_error(ret, __FILENAME__, __LINE__);
			return -1;
		}
		sd_titles = malloc(num_sdtitles*sizeof(u64));
		ret = AM_GetTitleList(NULL, mediatype, num_sdtitles, sd_titles);
		if(ret){
			gfx_error(ret, __FILENAME__, __LINE__);
			return -1;
		}
		titleid = 1337LL;
		for(i = 0; i < (signed)num_sdtitles && titleid == 1337LL; i++){
			if(sd_titles[i] == NA_GAMEID)
				titleid = NA_GAMEID;
			else if(sd_titles[i] == EU_GAMEID)
				titleid = EU_GAMEID;
			else if(sd_titles[i] == JP_GAMEID)
				titleid = JP_GAMEID;
			if(titleid != 1337LL){
				gfx_displaymessage("Found!");
			}
		}
		if(titleid == 1337LL){
			gfx_waitmessage("Error! Could not find a copy of AC:NL on this device!");
			return -1;
		}
	}
	amExit();
	if(titleid != NA_GAMEID && titleid != EU_GAMEID && titleid != JP_GAMEID){
		gfx_waitmessage("Unknown titleID 0x%llX", titleid);
		return -1;
	}
	lowerid = (u32)(titleid);
	upperid = (u32)(titleid >> 32);
	uniqueid = (lowerid >> 8);
	
	return 0;
}
