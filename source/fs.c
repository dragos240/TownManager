#include <string.h>
#include <stdlib.h>

#include <3ds.h>

#include "common.h"
#include "gfx.h"
#include "fs.h"

FS_Archive game_arch, sdmc_arch;

Result fs_init(){
	Handle fs_handle;
	Result ret;
	u64 titleid;
	u32 lowerid, upperid;
	u32* path;
	FS_Path null_binpath, game_binpath;

	if(is3dsx){
		fsInit();
		ret = srvGetServiceHandleDirect(&fs_handle, "fs:USER");
		if(ret)
			return ret;
		ret = FSUSER_Initialize(fs_handle);
		if(ret)
			return ret;
		fsUseSession(fs_handle);
	}
	
	//open sdmc card
	null_binpath = (FS_Path){PATH_EMPTY, 1, (u8*)""};
	ret = FSUSER_OpenArchive(&sdmc_arch, ARCHIVE_SDMC, null_binpath);
	if(ret)
		return ret;

	amInit();
	AM_GetTitleList(NULL, MEDIATYPE_GAME_CARD, 1, &titleid);
	if(titleid != NA_GAMEID && titleid != EU_GAMEID && titleid != JP_GAMEID){
		gfx_waitmessage("Unknown titleID 0x%llX", titleid);
		return -1;
	}
	lowerid = (u32)(titleid);
	upperid = (u32)(titleid >> 32);
	if(is3dsx){
		ret = FSUSER_OpenArchive(&game_arch, ARCHIVE_SAVEDATA, fsMakePath(PATH_EMPTY, ""));
		if(ret)
			return ret;
	}
	else{
		path = (u32[3]){2, lowerid, upperid};
		game_binpath = (FS_Path){PATH_BINARY, 12, path};
		ret = FSUSER_OpenArchive(&game_arch, ARCHIVE_USER_SAVEDATA, game_binpath);
		if(ret)
			return ret;
	}

	return ret;
}

Result fs_fini(){
	Result ret;

	ret = FSUSER_CloseArchive(game_arch);
	ret |= FSUSER_CloseArchive(sdmc_arch);

	if(is3dsx){
		fsEndUseSession();
		fsExit();
	}
	amExit();

	return ret;
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
