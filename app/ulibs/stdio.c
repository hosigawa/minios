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

int fgets(int fd, char *dst, int len)
{
	int ret;
	int target = len;
	char c;
	while((ret = read(fd, &c, 1)) != 0 && len-- > 0) {
		*dst = c;
		if(c == '\n'){
			*dst = 0;
			break;
		}
		dst++;
	}
	return target - len;
}

int gets(char *dst, int len)
{
	return fgets(stdin, dst, len);
}

