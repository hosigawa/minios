#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "kbd.h"

void cprintfint(int data, int base, bool sign);
void cprintfarg(char *fmt, uint *argp);
void cprintf(char *fmt, ...);
void panic(char *fmt, ...);
void cputc(int c);
void init_console();
void console_proc(int data);

#endif

