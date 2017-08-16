#include "crt.h"
#include "x86.h"

void printf(char *fmt, ...)
{
	uint *argp = (uint*)(&fmt + 1);
	vprintf(fmt, argp);
}

void *memset(void *dst, int data, int len)
{
	stosb(dst, data, len);
	return dst;
}

