#ifndef __UNISTD_H__
#define __UNISTD_H__

#include "libc.h"
#include "usyscall.h"
#include "signal.h"

#define TIME_ZONE 8

void crt_init();
void crt_fini();
int exec(char *path, char **argv, char **envp);
void srand(int seek);
int rand();
int localtime(uint time, struct time_v *tm);
int gettimeofday(struct time_v *tm);
char *invert_strcpy(char *dst, char *src, int len);

#endif

