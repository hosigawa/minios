#include "stdio.h"
#include "malloc.h"

#define GET_VER_H(v) (((v)>>16)&0xff)
#define GET_VER_M(v) (((v)>>8)&0xff)
#define GET_VER_L(v) ((v)&0xff)

int next_int(char **p)
{
	int sum = 0;
	while(**p != ' ' && **p != '\n') 
		sum = sum * 10 + *((*p)++) - '0';

	while(**p == ' ' || **p == '\n')
		(*p)++;

	return sum;
}

char *next_str(char *dst, char **p)
{
	while(**p != ' ' && **p != '\n')
		*dst++ = *((*p)++);
	*dst = 0;

	while(**p == ' ' || **p == '\n')
		(*p)++;

	return dst;
}

int main()
{
	char sys_info[4096];
	memset(sys_info, 0, 4096);
	int fd = open("/dev/sys", 0);
	if(fd < 0) {
		printf("ps: open /dev/sys error\n");
		return -1;
	}
	uint ver;
	int mt,mf,ut,kt,tt;
	char *p = sys_info;
	read(fd, sys_info, 4096);
	ver = (uint)next_int(&p);
	mt = next_int(&p);
	mf = next_int(&p);
	ut = next_int(&p);
	kt = next_int(&p);
	tt = next_int(&p);

	printf("Version      : %d.%d.%d\n", GET_VER_H(ver), GET_VER_M(ver), GET_VER_L(ver));
	printf("Memory Total : %d kB\n", mt / 1024);
	printf("Memory Free  : %d kB\n", mf / 1024);
	printf("User Time    : %d:%02d:%02d(%d)\n", 
							ut / 1000 / 3600 % 24,
							ut / 1000 / 60 % 60,
							ut / 1000 % 60,
							ut);
	printf("Kern Time    : %d:%02d:%02d(%d)\n", 
							kt / 1000 / 3600 % 24,
							kt / 1000 / 60 % 60,
							kt / 1000 % 60,
							kt);
	printf("Total Time   : %d:%02d:%02d(%d)\n", 
							tt / 1000 / 3600 % 24,
							tt / 1000 / 60 % 60,
							tt / 1000 % 60,
							tt);
	close(fd);
	return 0;
}

