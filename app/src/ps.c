#include "stdio.h"
#include "malloc.h"

static char *STATUS[] = {
	"U ", 
	"E ",
	"R-",
	"R+",
	"S ",
	"Z ",
};

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
	char proc_info[4096];
	memset(proc_info, 0, 4096);
	int fd = open("/dev/proc", 0);
	if(fd < 0) {
		printf("ps: open /dev/proc error\n");
		return -1;
	}
	read(fd, proc_info, 4096);
	printf(" PID  PPID    VSZ  STAT     TIME  CMD\n");
	printf("-------------------------------------\n");
	char *p = proc_info;
	int pid;
	int ppid;
	uint vsz;
	int st;
	uint ticks;
	uint min, sec, ms;
	char nm[64];
	while(*p) {
		pid = next_int(&p);
		ppid = next_int(&p);
		vsz = next_int(&p);
		st = next_int(&p);
		ticks = next_int(&p);
		min = (ticks / 1000) / 60;
		sec = (ticks / 1000) % 60;
		ms = (ticks % 1000) / 100;
		memset(nm, 0, 64);
		next_str(nm, &p);
		printf("%4d  %4d  %5d  %s  %4d:%02d.%d  %s\n", pid, ppid, vsz, STATUS[st], min, sec, ms, nm);
	}
	close(fd);
	return 0;
}

