#ifndef __LIBC_H__
#define __LIBC_H__

#include "../kernel/type.h"

#define abs(x) ((x) > 0 ? (x) : -(x))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

void *memmove(void *dst, void *src, int len);
void *memset(void *dst, int data, int len);
int strcmp(const char *dst, const char *src);
int strncmp(const char *dst, const char *src, int len);
int strlen(char *src);
int sprintf(char *dst, char *fmt, ...);
typedef void (*cputc)(char *dst, int data);
void vprintf(char *fmt, uint *argp, char *dst, cputc putc);
char *vprintfint(int data, int base, bool sign, char *dst, cputc putc);

#endif

