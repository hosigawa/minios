#include "stdio.h"

static void stdout_putc(char *dst, int c)
{
	write(stdout, (char *)&c, 1);
}

void printf(char *fmt, ...)
{
	uint *argp = (uint*)(&fmt + 1);
	vprintf(fmt, argp, NULL, stdout_putc);
}

