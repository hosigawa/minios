#ifndef __CRT_H__
#define __CRT_H__

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

void *memmove(void *dst, void *src, int len);
void *memset(void *dst, int data, int len);
int strncmp(const char *dst, const char *src, int len);
void vprintf(char *fmt, uint *argp);
void cprintf(char *fmt, ...);
void panic(char *fmt, ...);
void cprintfint(int data, int base, bool sign);
void cputc(int c);

#endif

