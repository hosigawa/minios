#ifndef __CRT_H__
#define __CRT_H__

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

void *memmove(void *dst, void *src, int len);
void *memset(void *dst, int data, int len);
int strcmp(const char *dst, const char *src);
int strncmp(const char *dst, const char *src, int len);
int strlen(char *src);

#endif

