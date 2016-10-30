#ifndef TM_BACKUP_H
#define TM_BACKUP_H

u64 filesize_to_u64(FS_Archive arch, char* filepath);
int buffer_to_file(FS_Archive arch, char* buffer, u64 size, char* dirpath, char* filename);
char* file_to_buffer(FS_Archive arch, char* dirpath, char* filename);

void backup_to_prev_folder(char* dirname);

#endif
