#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <3ds.h>

#include "common.h"
#include "fs.h"
#include "gfx.h"
#include "backup.h"
#include "launcher.h"

static Handle nssHandle = 0;

Result launcher_init(){
	Result ret;
	ret = srvGetServiceHandle(&nssHandle, "ns:s");

	return ret;
}

Result launcher_fini(){
	return svcCloseHandle(nssHandle);
}

int prepare_to_launch(char* dirname){
	Handle handle;
	char* savespath = "/TownManager/Saves/";
	char* dirpath;
	char* filepath;
	char* buf;
	u32 written;
	u64 size;
	file_t files;
	int i;
	int ret;

	gfx_displaymessage("Preparing to launch game...");
	dirpath = calloc(strlen(savespath)+strlen(dirname)+1+1, 1);
	sprintf(dirpath, "%s%s/", savespath, dirname);
	files = get_files(sdmc_arch, dirpath);

	//copy local files to cartridge
	for(i = 0; i < files.numfiles; i++){
		buf = file_to_buffer(sdmc_arch, dirpath, files.files[i]);
		if(buf == NULL){
			gfx_waitmessage("Buffer is empty!");
			return -1;
		}
		filepath = calloc(strlen(dirpath)+strlen(files.files[i])+1, 1);
		sprintf(filepath, "%s%s", dirpath, files.files[i]);
		size = filesize_to_u64(sdmc_arch, filepath);
		ret = buffer_to_file(game_arch, buf, size, "/", files.files[i]);
		if(ret == -1){
			gfx_waitmessage("buffer_to_file failed!");
			return -1;
		}
	}
	free(dirpath);
	//Write dirname as the sole contents of the /TownManager/tm.conf file
	ret = FSUSER_OpenFile(&handle, sdmc_arch, fsMakePath(PATH_ASCII, "/TownManager/tm.conf"), FS_OPEN_WRITE, 0);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		return -1;
	}
	ret = FSFILE_Write(handle, &written, 0, dirname, strlen(dirname)+1, FS_WRITE_FLUSH | FS_WRITE_UPDATE_TIME);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		return -1;
	}
	ret = FSFILE_SetSize(handle, (u64)(strlen(dirname)+1));
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		return -1;
	}
	ret = FSFILE_Close(handle);
	if(ret){
		gfx_error(ret, __FILENAME__, __LINE__);
		return -1;
	}

	return 0;
}

Result NSS_Reboot(u32 pid_low, u32 pid_high, u8 mediatype, u8 flag){
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00100180;
	cmdbuf[1] = flag;
	cmdbuf[2] = pid_low;
	cmdbuf[3] = pid_high;
	cmdbuf[4] = mediatype;
	cmdbuf[5] = 0x00000000;
	cmdbuf[6] = 0x00000000;

	if((ret = svcSendSyncRequest(nssHandle))!=0) return ret;

	return (Result)cmdbuf[1];
}

void launch_game(){
	Result ret;

	if(is3dsx){
		ret = NSS_Reboot(lowerid, upperid, mediatype, 0x1);
		if(ret){
			gfx_error(ret, __FILE__, __LINE__);
		}
	}
	else{
		u8 param[0x300];
		u8 hmac[0x20];
		memset(param, 0, sizeof(param));
		memset(hmac, 0, sizeof(hmac));

		APT_PrepareToDoApplicationJump(0, titleid, mediatype);
		APT_DoApplicationJump(param, sizeof(param), hmac);
	}
}
