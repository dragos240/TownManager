#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <3ds.h>

#include "common.h"
#include "gfx.h"
#include "menu.h"
#include "conf.h"
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

Result open_sdmc_archive(){
	Result ret;

	//open sdmc card
	ret = FSUSER_OpenArchive(&sdmc_arch, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
	if(ret)
		return ret;

	return ret;
}

Result open_game_archive(){
	Result ret;
	u32* path;
	FS_Path game_binpath;

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

int load_config(char** current_town){
	//loads config file and sets *current_town
	Result ret;
	Handle handle;
	FILE* f;
	u64 size;
	int tempsize;
	u32 read; //bytes read
	u32 written; //bytes written
	char* buf;
	conftok_t token;
	file_t files;
	const char* configpath = "/TownManager/config";
	int i;
	bool tm_conf_exists = false;

	//if config file was just created now
	if((ret = FSUSER_CreateFile(sdmc_arch, fsMakePath(PATH_ASCII, configpath), 0, 0)) == 0){
		//look for existence of tm.conf (the old config file)
		files = get_files(sdmc_arch, "/TownManager/");
		for(i = 0; i < files.numfiles; i++){
			if(strcmp(files.files[i], "tm.conf") == 0){
				tm_conf_exists = true;
				if((ret = FSUSER_OpenFile(&handle, sdmc_arch, fsMakePath(PATH_ASCII, "/TownManager/tm.conf"), FS_OPEN_READ, 0))){
					gfx_error(ret, __FILENAME__, __LINE__);
					return -1;
				}
				if((ret = FSFILE_GetSize(handle, &size))){
					gfx_error(ret, __FILENAME__, __LINE__);
					return -1;
				}
				buf = malloc(size);
				if((ret = FSFILE_Read(handle, &read, 0, buf, (u32)size))){
					gfx_error(ret, __FILENAME__, __LINE__);
					return -1;
				}
				token.townname = calloc((int)size, 1);
				//using strncpy because buf will be freed soon
				strncpy(token.townname, buf, size);
				free(buf);
				//get mediatype...
				get_mediatype();
				//and then assign its value to token.mediatype_val
				token.mediatype_val = mediatype;
				size = 1+size+1; //no need for another +1 because size includes '\0' already
				buf = calloc(size, 1);
				conf_gen(&buf, &token);
				if((ret = FSFILE_Close(handle))){
					gfx_error(ret, __FILENAME__, __LINE__);
					return -1;
				}
			}
		}
		//open the newly created config file
		if((ret = FSUSER_OpenFile(&handle, sdmc_arch, fsMakePath(PATH_ASCII, configpath), FS_OPEN_READ | FS_OPEN_WRITE, 0))){
			gfx_error(ret, __FILENAME__, __LINE__);
			return -1;
		}
		if(tm_conf_exists == false){
			//open template config file
			if(!(f = fopen("romfs:/config", "r"))){
				gfx_waitmessage("Could not open template config file.");
				return -1;
			}
			fseek(f, 0L, SEEK_END);
			tempsize = ftell(f);
			rewind(f);
			if((buf = malloc(tempsize)) == NULL){
				gfx_waitmessage("malloc failed! %s:%d", __FILENAME__, __LINE__);
				return -1;
			}
			fread(buf, tempsize, 1, f);
			fclose(f);

			conf_parse(buf, &token);
			get_mediatype();
			token.mediatype_val = mediatype;
			free(buf);
			buf = calloc(2+strlen(token.townname)+1+1, 1);
			conf_gen(&buf, &token);
			size = strlen(buf);
			if((ret = FSFILE_Write(handle, &written, 0, buf, size, FS_WRITE_FLUSH | FS_WRITE_UPDATE_TIME))){
				gfx_error(ret, __FILENAME__, __LINE__);
				return -1;
			}
			if(size != written){
				gfx_waitmessage("size != written! %s:%d", __FILENAME__, __LINE__);
				return -1;
			}
			if((ret = FSFILE_SetSize(handle, size))){
				gfx_error(ret, __FILENAME__, __LINE__);
				return -1;
			}
			if((ret = FSFILE_Close(handle))){
				gfx_error(ret, __FILENAME__, __LINE__);
				return -1;
			}
			*current_town = "Main";
			return 0;
		}
		//else if tm.conf exists...
		else{
			if((ret = FSFILE_Write(handle, &written, 0, buf, size, FS_WRITE_FLUSH | FS_WRITE_UPDATE_TIME))){
				gfx_error(ret, __FILENAME__, __LINE__);
				return -1;
			}
			if(size != written){
				gfx_waitmessage("size != written! %s:%d", __FILENAME__, __LINE__);
				return -1;
			}
			if((ret = FSFILE_SetSize(handle, size))){
				gfx_error(ret, __FILENAME__, __LINE__);
				return -1;
			}
			if((ret = FSFILE_Close(handle))){
				gfx_error(ret, __FILENAME__, __LINE__);
				return -1;
			}
			*current_town = token.townname;
			return 0;
		}
	}
	//if config file exists
	else{
		if((ret = FSUSER_OpenFile(&handle, sdmc_arch, fsMakePath(PATH_ASCII, configpath), FS_OPEN_READ, 0))){
			gfx_error(ret, __FILENAME__, __LINE__);
			return -1;
		}
		if((ret = FSFILE_GetSize(handle, &size))){
			gfx_error(ret, __FILENAME__, __LINE__);
			return -1;
		}
		if((buf = malloc(size)) == NULL){
			gfx_waitmessage("malloc failed! %s:%d", __FILENAME__, __LINE__);
			return -1;
		}
		if((ret = FSFILE_Read(handle, &read, 0, buf, size))){
			gfx_error(ret, __FILENAME__, __LINE__);
			return -1;
		}
		if((ret = FSFILE_Close(handle))){
			gfx_error(ret, __FILENAME__, __LINE__);
			return -1;
		}
		conf_parse(buf, &token);
		mediatype = token.mediatype_val;
		*current_town = token.townname;
	}

	return 0;
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
	FSDIR_Close(handle);

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
	FSDIR_Close(handle);

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

void change_mediatype(){
	Result ret;
	Handle handle;
	u32 read;
	u32 written;
	u32 size;
	char* buf;
	conftok_t token;

	get_mediatype();
	if((ret = FSUSER_OpenFile(&handle, sdmc_arch, fsMakePath(PATH_ASCII, "/TownManager/config"), FS_OPEN_READ | FS_OPEN_WRITE, 0))){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	if((ret = FSFILE_GetSize(handle, (u64*)&size))){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	buf = malloc(size);
	if((ret = FSFILE_Read(handle, &read, 0, buf, size))){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	conf_parse(buf, &token);
	token.mediatype_val = mediatype;
	conf_gen(&buf, &token);
	if((ret = FSFILE_Write(handle, &written, 0, buf, size, FS_WRITE_FLUSH | FS_WRITE_UPDATE_TIME))){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	if((ret = FSFILE_Close(handle))){
		gfx_error(ret, __FILENAME__, __LINE__);
		return;
	}
	
	gfx_waitmessage("Changed media source.");
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
			else if(sd_titles[i] == NA_WA_GAMEID)
				titleid = NA_WA_GAMEID;
			else if(sd_titles[i] == EU_GAMEID)
				titleid = EU_GAMEID;
			else if(sd_titles[i] == EU_WA_GAMEID)
				titleid = EU_WA_GAMEID;
			else if(sd_titles[i] == JP_GAMEID)
				titleid = JP_GAMEID;
			else if(sd_titles[i] == JP_WA_GAMEID)
				titleid = JP_WA_GAMEID;
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
	if(titleid != NA_GAMEID && titleid != NA_WA_GAMEID &&
			titleid != EU_GAMEID && titleid != EU_WA_GAMEID &&
			titleid != JP_GAMEID && titleid != JP_WA_GAMEID){
		gfx_waitmessage("Unknown titleID 0x%llX", titleid);
		return -1;
	}
	lowerid = (u32)(titleid);
	upperid = (u32)(titleid >> 32);
	uniqueid = (lowerid >> 8);
	
	return 0;
}
