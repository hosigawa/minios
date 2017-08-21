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

int strcmp(const char *dst, const char *src)
{
	do {
		if(*dst++ != *src++)
			return -1;
	} while(*src || *dst);
	return 0;
}

int strncmp(const char *dst, const char *src, int len)
{
	int i;
	for(i = 0; i < len; i++) {
		if(*dst++ != *src++)
			return -1;
		if(*dst == 0)
			return -1;
	}
	return 0;
}

