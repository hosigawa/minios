#ifndef __STDIO_H__
#define __STDIO_H__

#include "usyscall.h"
#include "../libs/libc.h"

#define stdin 0
#define stdout 1
#define stderr 2

void printf(char *fmt, ...);
int fgets(int fd, char *dst, int len);

#endif

