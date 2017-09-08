#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "kbd.h"
#include "fs.h"

#define BACKSPACE 0x100
#define INPUT_BUFF 128

void cprintf(char *fmt, ...);
void panic(char *fmt, ...) __attribute__((noreturn));
void console_putc(int c);
void init_console();
void console_proc(int (*getc)(void));

int console_read(struct inode *ip, char *dst, int off, int len);
int console_write(struct inode *ip, char *dst, int off, int len);

#endif

