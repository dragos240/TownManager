#ifndef TM_FS_H
#define TM_FS_H

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
Result fs_fini();

dir_t get_dirs(char* path);
file_t get_files(FS_Archive arch, char* path);

#endif
