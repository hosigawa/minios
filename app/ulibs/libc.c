#include "libc.h"
#include "x86.h"

void printf(char *fmt, ...)
{
	uint *argp = (uint*)(&fmt + 1);
	cprintfarg(stdout, fmt, argp);
}

void *memset(void *dst, int data, int len)
{
	stosb(dst, data, len);
	return dst;
}

void cprintfint(int fd, int data, int base, bool sign) 
{
	static char digitals[] = "0123456789ABCDEF";
	char buf[32] = {0};

	uint abs_data = 0;
	if(sign && (sign = data < 0))
		abs_data = -data;
	else
		abs_data = data;

	int i = 0;
	do {
		buf[i++] = digitals[abs_data % base];
	}while((abs_data /= base) != 0);

	if(base == 16) {
		buf[i++] = 'x';
		buf[i++] = '0';
	}
	if(sign)
		buf[i++] = '-';

	while(--i >= 0)
		cputc(fd, buf[i]);
}

void cprintfarg(int fd, char *fmt, uint *argp) 
{
	int c;
	char *s;
	int i = 0;
	for(;(c = fmt[i]&0xff) != 0; i++) {
		if(c != '%'){
			cputc(fd, c);
			continue;
		}
		c = fmt[++i]&0xff;
		if(c == 0)
			break;
		switch(c) {
			case 's':
				if((s = (char *)*argp++) == 0){
					s= "(null)";
				}
				for(; *s != 0; s++) {
					cputc(fd, *s);
				}
			break;
			case 'd':
				cprintfint(fd, *argp++, 10, true);
			break;
			case 'x':
			case 'p':
				cprintfint(fd, *argp++, 16, false);
			break;
			case '%':
				cputc(fd, '%');
			break;
			default:
				cputc(fd, '%');
				cputc(fd, c);
			break;
		}
	}
}

void cputc(int fd, char c)
{
	write(fd, &c, 1);
}

