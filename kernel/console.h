#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "kbd.h"
#include "fs.h"

#define BACKSPACE 0x100
#define INPUT_BUFF 128

void cprintfint(int data, int base, bool sign);
void cprintfarg(char *fmt, uint *argp);
void cprintf(char *fmt, ...);
void panic(char *fmt, ...) __attribute__((noreturn));
void console_putc(int c);
void init_console();
void console_proc(int data);

int console_read(struct inode *ip, char *dst, int len);
int console_write(struct inode *ip, char *dst, int len);

#endif

