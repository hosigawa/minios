#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "kbd.h"
#include "fs.h"

void cprintfint(int data, int base, bool sign);
void cprintfarg(char *fmt, uint *argp);
void cprintf(char *fmt, ...);
void panic(char *fmt, ...) __attribute__((noreturn));
void cputc(int c);
void init_console();
void console_proc(int data);

int console_read(struct inode *ip, char *dst, int len);
int console_write(struct inode *ip, char *dst, int len);

#endif

