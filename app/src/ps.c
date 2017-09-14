#include "stdio.h"
#include "malloc.h"

static char *STATUS[] = {
	"U ", 
	"E ",
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
	while(**p != '\n')
		*dst++ = *((*p)++);
	*dst = 0;

	while(**p == '\n')
		(*p)++;

	return dst;
}

static bool name_is_digital(char *name)
{
	char *p = name;
	while(*p) {
		if(!isdigit(*p))
			return false;
		p++;
	}
	return true;
}

int main(int argc, char *argv[])
{
	char info[4096];
	char real_path[64] = {0};
	int fd = open("/proc", 0);
	if(fd < 0) {
		printf("ps: open /proc error\n");
		return -1;
	}
	struct dirent de[256];
	memset(de, 0, 256 * sizeof(struct dirent));
	int ret = readdir(fd, de);
	close(fd);
	int i;
	int sfd;
	printf(" PID  PPID     VSZ PRI  STAT     TIME  CMD\n");
	printf("------------------------------------------\n");
	for(i = 0; i < ret; i++) {
		if(!name_is_digital(de[i].name)) {
			continue;
		}
		memset(real_path, 0, 64);
		sprintf(real_path, "/proc/%s/stat", de[i].name);
		sfd = open(real_path, 0);
		if(sfd < 0)
			continue;
		memset(info, 0, 4096);
		read(sfd, info, 4096);
		close(sfd);
		char *p = info;
		int pid = 0;
		int ppid = 0;
		uint vsz = 0;
		int pri = 0;
		int st = 0;
		uint ticks = 0;
		uint min = 0, sec = 0, ms = 0;
		while(*p) {
			pid = next_int(&p);
			ppid = next_int(&p);
			vsz = next_int(&p);
			st = next_int(&p);
			ticks = next_int(&p);
			min = (ticks / 1000) / 60;
			sec = (ticks / 1000) % 60;
			ms = (ticks % 1000) / 100;
			pri = next_int(&p);
		}

		char nm[64];
		memset(real_path, 0, 64);
		sprintf(real_path, "/proc/%s/cmdline", de[i].name);
		sfd = open(real_path, 0);
		if(sfd < 0)
			continue;
		memset(info, 0, 4096);
		read(sfd, info, 4096);
		close(sfd);
		p = info;
		memset(nm, 0, 64);
		strcpy(nm, p);

		printf("%4d  %4d  %5d  %2d  %s  %4d:%02d.%d  %s\n", pid, ppid, vsz, pri, STATUS[st], min, sec, ms, nm);
	}
	return 0;
}

