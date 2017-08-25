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

void vsprintf(char *dst, char *fmt, uint *argp)
{
	int c;
	char *s;
	int i = 0;
	for(;(c = fmt[i]&0xff) != 0; i++) {
		if(c != '%'){
			dst++ = c;
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
					dst++ = *s;
				}
			break;
			case 'd':
				dst = cprintfint(dst, *argp++, 10, true);
			break;
			case 'x':
			case 'p':
				dst = cprintfint(dst, *argp++, 16, false);
			break;
			case '%':
				dst++ = '%';
			break;
			default:
				dst++ = '%';
				dst++ = c;
			break;
		}
	}
}

char *vsprintfint(char *dst, int data, int base, bool sign) 
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
		dst++ = buf[i];
	return dst;
}

int sprintf(char *dst, char *fmt, ...)
{
	uint *argp = (uint*)(&fmt + 1);
	vsprintf(dst, fmt, argp);
	return 0;
}

