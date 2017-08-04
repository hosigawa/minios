#ifndef __CRT_H__
#define __CRT_H__

void *memmove(void *dst, void *src, int len);
void *memset(void *dst, int data, int len);
void vprintf(char *fmt, uint *argp);
void cprintf(char *fmt, ...);
void panic(char *fmt, ...);
void cprintfint(int data, int base, bool sign);
void cputc(int c);

#endif

