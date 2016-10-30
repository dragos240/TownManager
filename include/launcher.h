#ifndef TM_LAUNCHER_H
#define TM_LAUNCHER_H

Result launcher_init();
Result launcher_fini();

int prepare_to_launch(char* dirname);
Result NSS_Reboot(u32 pid_low, u32 pid_high, u8 mediatype, u8 flag);
void launch_game();

#endif
