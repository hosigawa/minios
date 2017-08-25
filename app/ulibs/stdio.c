#include "stdio.h"

void printf(char *fmt, ...)
{
	uint *argp = (uint*)(&fmt + 1);
	cprintfarg(fmt, argp, stdout_putc);
}

void stdout_putc(int c)
{
	write(stdout, (char *)&c, 1);
}

