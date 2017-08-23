#ifndef __STDIO_H__
#define __STDIO_H__

#include "usyscall.h"
#include "../libs/libc.h"

#define stdin 0
#define stdout 1
#define stderr 2

void printf(char *fmt, ...);
void cputc(int fd, char c);
void cprintfarg(int fd, char *fmt, uint *argp); 
void cprintfint(int fd, int data, int base, bool sign);

#endif

