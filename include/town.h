#ifndef TM_TOWN_H
#define TM_TOWN_H

void populate_menu_entries(char*** menu_entries, int* menucount);

void create_town();
void init_save_folder();
void load_tm_config(char** current);

void town_opts(char* dirname);
void clone_town(char* dirname);
void rename_town(char* dirname);
void delete_town(char* dirname);

#endif
