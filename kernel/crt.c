#include "kernel.h"

void *memmove(void *dst, void *src, int len) 
{
	char *d = dst;
	char *s = src;;
	while(len-- > 0)
		*d++ = *s++;
	return dst;
}

void *memset(void *dst, int data, int len) 
{
	stosb(dst, data, len);
	return dst;
}

int strncmp(const char *dst, const char *src, int len)
{
	do {
		if(*dst++ != *src++)
			return -1;
	} while(*dst && *src && --len > 0);
	return 0;
}

