#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>

#include "common.h"
#include "gfx.h"
#include "fs.h"
#include "backup.h"

u64 filesize_to_u64(FS_Archive arch, char* filepath){
	u64 file_size;
	Result ret;
	Handle file_handle;

	//open file
	ret = FSUSER_OpenFile(&file_handle, arch, fsMakePath(PATH_ASCII, filepath), FS_OPEN_READ, 0);
	if(ret)
		gfx_error(ret, __FILENAME__, __LINE__);

	//get size off file
	FSFILE_GetSize(file_handle, &file_size);

	//close handle
	ret = FSFILE_Close(file_handle);
	if(ret)
		gfx_error(ret, __FILENAME__, __LINE__);

	return file_size;
}

int buffer_to_file(FS_Archive arch, char* buffer, u64 size, char* dirpath, char* filename){
	int error = 0;
	u32 written; //bytes written
	char* file_out;
	Result ret;
	Handle file_handle;

	//Writing
	file_out = calloc(strlen(dirpath)+strlen(filename)+1, 1);
	sprintf(file_out, "%s%s", dirpath, filename);

	ret = FSUSER_OpenFile(&file_handle, arch, fsMakePath(PATH_ASCII, file_out), FS_OPEN_WRITE | FS_OPEN_CREATE, 0);
	free(file_out);
	if(ret){
	  gfx_error(ret, __FILENAME__, __LINE__);
	  error = 1;
	  goto btf_close_files;
	}

	ret = FSFILE_Write(file_handle, &written, 0, buffer, size, FS_WRITE_FLUSH | FS_WRITE_UPDATE_TIME);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		error = 1;
		goto btf_close_files;
	}
	if(written != size){
		gfx_waitmessage("local %s incorrectly written to!", filename);
		error = 1;
		goto btf_close_files;
	}
	if(arch == game_arch){
		ret = FSUSER_ControlArchive(game_arch, ARCHIVE_ACTION_COMMIT_SAVE_DATA, NULL, 0, NULL, 0);
		if(ret){
			gfx_error(ret, __FILENAME__, __LINE__);
			error = 1;
		}
	}

btf_close_files:

	ret = FSFILE_Close(file_handle);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		error = 1;
	}
	if(error == 1){
		return -1;
	} else
		return 0;
}

char* file_to_buffer(FS_Archive arch, char* dirpath, char* filename){
	int error = 0;
	u32 read; //bytes read/written
	u64 file_size;
	char* file_in;
	char* buffer = NULL;
	Result ret;
	Handle file_handle;

	file_in = calloc(strlen(dirpath)+strlen(filename)+1, 1);
	sprintf(file_in, "%s%s", dirpath, filename);

	//open file
	ret = FSUSER_OpenFile(&file_handle, arch, fsMakePath(PATH_ASCII, file_in), FS_OPEN_READ, 0);
	free(file_in);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		error = 1;
		goto ftb_close_files;
	}

	//get size of file
	ret = FSFILE_GetSize(file_handle, &file_size);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		error = 1;
		goto ftb_close_files;
	}

	//allocate space for file in buffer
	buffer = malloc(file_size+1);
	if(!buffer){
		gfx_waitmessage("malloc failed!");
		error = 1;
		goto ftb_close_files;
	}

	//read file into buffer
	ret = FSFILE_Read(file_handle, &read, 0, buffer, file_size);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		error = 1;
		goto ftb_close_files;
	}
	if(read != file_size){
		gfx_waitmessage("Problem reading from %s!", filename);
		error = 1;
		goto ftb_close_files;
	}

	ftb_close_files:

	ret = FSFILE_Close(file_handle);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		error = 1;
	}
	if(error == 1){
		gfx_waitmessage("Uh oh! Errors occurred while running file_to_buffer!");
		free(buffer);
		return NULL;
	}
	else
		return buffer;
}

void backup_to_prev_folder(char* dirname){
	Result ret;
	file_t files;
	u64 size;
	char* gamepath = "/";
	char* savespath = "/TownManager/Saves/";
	char* sdmcpath;
	char* filepath;
	char* buf;
	int error = 0;
	int i;

	FSUSER_CreateDirectory(sdmc_arch, fsMakePath(PATH_ASCII, savespath), 0);
	//Erase sdmc data
	sdmcpath = calloc(strlen(savespath)+strlen(dirname)+1+1, 1);
	sprintf(sdmcpath, "%s%s/", savespath, dirname);
	FSUSER_CreateDirectory(sdmc_arch, fsMakePath(PATH_ASCII, sdmcpath), 0);
	files = get_files(sdmc_arch, sdmcpath);
	if(files.numfiles != 0){
		gfx_displaymessage("Erasing old files from the SD card...");
		for(i = 0; i < files.numfiles; i++){
			filepath = calloc(strlen(savespath)+strlen(dirname)+1+strlen(files.files[i])+1, 1);
			sprintf(filepath, "%s%s/%s", savespath, dirname, files.files[i]);
			gfx_displaymessage("Deleting %s from SD card...", files.files[i]);
			ret = FSUSER_DeleteFile(sdmc_arch, fsMakePath(PATH_ASCII, filepath));
			if(ret){
				gfx_error(ret, __FILENAME__, __LINE__);
				gfx_waitmessage("dirname: %s", dirname);
				return;
			}
			free(filepath);
		}
	}
	free(sdmcpath);
	//backup saves
	sdmcpath = calloc(strlen(savespath)+strlen(dirname)+1+1, 1);
	sprintf(sdmcpath, "%s%s/", savespath, dirname);
	files = get_files(game_arch, "/");
	for(i = 0; i < files.numfiles; i++){
		gfx_displaymessage("Backing up %s to %s...", files.files[i], dirname);
		buf = file_to_buffer(game_arch, gamepath, files.files[i]);
		if(!buf){
			gfx_waitmessage("Error: buffer empty!");
			break;
		}
		filepath = calloc(strlen(gamepath)+strlen(files.files[i])+1, 1);
		sprintf(filepath, "%s%s", gamepath, files.files[i]);
		size = filesize_to_u64(game_arch, filepath);
		error = buffer_to_file(sdmc_arch, buf, size, sdmcpath, files.files[i]);
		if(error == -1){
			gfx_waitmessage("buffer_to_file failed!");
		}
		free(filepath);
		free(buf);
		if(error < 0)
			break;
	}
	free(sdmcpath);
	gfx_waitmessage("Done backing up game to %s", dirname);
}
