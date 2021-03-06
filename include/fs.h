#ifndef TM_FS_H
#define TM_FS_H

extern FS_MediaType mediatype;
extern u64 titleid;
extern u32 lowerid;
extern u32 upperid;
extern u32 uniqueid;

typedef struct{
	int numdirs;
	char** dirs;
} dir_t;

typedef struct{
	int numfiles;
	char** files;
} file_t;

extern FS_Archive game_arch;
extern FS_Archive sdmc_arch;

Result fs_init();
Result open_sdmc_archive();
Result open_game_archive();
Result close_archives();
void fs_fini();

int load_config(char** current_town);

dir_t get_dirs(char* path);
file_t get_files(FS_Archive arch, char* path);

void get_mediatype();
void change_mediatype();
int get_titleid();

#endif
