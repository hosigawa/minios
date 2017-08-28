#include "stdio.h"
#include "malloc.h"

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
	int mt,mf,it,i_f,ut,kt;
	char *p = sys_info;
	read(fd, sys_info, 4096);
	mt = next_int(&p);
	mf = next_int(&p);
	it = next_int(&p);
	i_f = next_int(&p);
	ut = next_int(&p);
	kt = next_int(&p);

	printf("Memory Total : %d kB\n", mt / 1024);
	printf("Memory Free  : %d kB\n", mf / 1024);
	printf("Inode Total  : %d\n", it);
	printf("Inode Free   : %d\n", i_f);
	printf("User Time    : %02d:%02d:%02d(%d)\n", 
							ut / 100 / 3600 % 24,
							ut / 100 / 60 % 60,
							ut / 100 % 60,
							ut);
	printf("Total Time   : %02d:%02d:%02d(%d)\n", 
							kt / 100 / 3600 % 24,
							kt / 100 / 60 % 60,
							kt / 100 % 60,
							kt);
	close(fd);
	return 0;
}

