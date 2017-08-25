#ifndef __LIBC_H__
#define __LIBC_H__

#include "type.h"

#define abs(x) ((x) > 0 ? (x) : -(x))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

void *memmove(void *dst, void *src, int len);
void *memset(void *dst, int data, int len);
int strcmp(const char *dst, const char *src);
int strncmp(const char *dst, const char *src, int len);
int strlen(char *src);
int sprintf(char *dst, char *fmt, ...);
void vsprintf(char *dst, char *fmt, uint *argp);
void vsprintfint(char *dst, int base, bool sign);

#endif

