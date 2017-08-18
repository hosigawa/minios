#ifndef __CRT_H__
#define __CRT_H__

#include "usyscall.h"

#define stdin 0
#define stdout 1
#define stderr 2

void printf(char *fmt, ...);
void *memset(void *dst, int data, int len);
void cputc(int fd, char c);
void cprintfarg(int fd, char *fmt, uint *argp); 
void cprintfint(int fd, int data, int base, bool sign);

#endif

