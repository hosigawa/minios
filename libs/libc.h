#ifndef __LIBC_H__
#define __LIBC_H__

#include "../kernel/type.h"
#include "time.h"

#define PAD0 0x1

#define abs(x) ((x) > 0 ? (x) : -(x))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define memmove memcpy

char *strcpy(char *dst, char *src);
void *memcpy(void *dst, void *src, int len);
//void *memmove(void *dst, void *src, int len);
void *memset(void *dst, int data, int len);
int strcmp(const char *dst, const char *src);
int strncmp(const char *dst, const char *src, int len);
int strlen(char *src);
int sprintf(char *dst, char *fmt, ...);
typedef void (*cputc)(char *dst, int data);
bool isdigit(char c);
char *vprintf(char *fmt, uint *argp, char *dst, cputc putc);
char *vprintfint(int data, int base, bool sign, int width, int flags, char *dst, cputc putc);
int atoi(char *a);

#endif

