#include "libc.h"
#include "../kernel/x86.h"
#include "../kernel/type.h"

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

int strlen(char *src)
{
	int len = 0;
	while(*src++)
		len++;
	return len;
}

static int skip_atoi(char *fmt, int *i)
{
	int sum = 0;
	while(isdigit(fmt[*i])) {
		sum = sum * 10 + fmt[(*i)++] - '0';
	}
	return sum;
}

char *vprintf(char *fmt, uint *argp, char *dst, cputc putc)
{
	int c;
	char *s;
	int i = 0;
	int width;
	for(;(c = fmt[i]&0xff) != 0; i++) {
		if(c != '%'){
			putc(dst++, c);
			continue;
		}
		i++;
		width = skip_atoi(fmt, &i);
		c = fmt[i]&0xff;
		if(c == 0)
			break;
		switch(c) {
			case 's':
				if((s = (char *)*argp++) == 0){
					s= "(null)";
				}
				for(; *s != 0; s++) {
					putc(dst++, *s);
				}
			break;
			case 'd':
				dst = vprintfint(*argp++, 10, true, width, dst, putc);
			break;
			case 'x':
			case 'p':
				dst = vprintfint(*argp++, 16, false, width, dst, putc);
			break;
			case '%':
				putc(dst++, '%');
			break;
			default:
				putc(dst++, '%');
				putc(dst++, c);
			break;
		}
	}
	return dst;
}

char *vprintfint(int data, int base, bool sign, int width, char *dst, cputc putc) 
{
	static char digitals[] = "0123456789ABCDEF";
	char buf[64] = {0};

	uint abs_data = 0;
	if(sign && (sign = data < 0))
		abs_data = -data;
	else
		abs_data = data;

	int i = 0;
	do {
		buf[i++] = digitals[abs_data % base];
		width--;
	}while((abs_data /= base) != 0);

	if(base == 16) {
		buf[i++] = 'x';
		buf[i++] = '0';
		width -= 2;
	}
	if(sign) {
		buf[i++] = '-';
		width--;
	}

	while(width > 0) {
		buf[i++] = ' ';
		width--;
	}

	while(--i >= 0)
		putc(dst++, buf[i]);
	return dst;
}

static void sprintf_putc(char *dst, int data)
{
	*dst = data;
}

int sprintf(char *dst, char *fmt, ...)
{
	uint *argp = (uint*)(&fmt + 1);
	char *nd = vprintf(fmt, argp, dst, sprintf_putc);
	return nd - dst;
}

bool isdigit(char c)
{
	if(c >= '0' && c <= '9')
		return true;
	return false;
}

