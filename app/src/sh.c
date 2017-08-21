#include "libc.h"

char wd[14];

int getcmd(char *buf, int len)
{
	int ret;
	memset(wd, 0, 14);
	if(pwd(wd) < 0)
		return -1;
	printf("[root@%s]$", wd);
	memset(buf, 0, len);
	ret = read(stdin, buf, len);
	if(ret <= 0)
		return -1;
	buf[ret - 1] = 0;
	return 0;
}

int get_token(char **argv, char *buf)
{
	int seq = 0;
	bool btk = false;
	while(*buf) {
		if(*buf == ' ' || *buf == '\t') {
			*buf = 0;
			btk = false;
		}
		else if(!btk) {
			argv[seq] = buf;
			seq++;
			btk = true;
		}
		buf++;
	}
	return 0;
}

int run_cmd(char *buf)
{
	char *argv[10] = {0};
	if(*buf == 0 || *buf == '\n')
		exit();
	get_token(argv, buf);
	exec(argv[0], argv);
	printf("-sh: %s: command not found\n", argv[0]);
	exit();
}

int main(int argc, char **argv) 
{
	printf("init sh...\n");
	char buf[100];

	while(getcmd(buf, sizeof(buf)) >= 0) {	
		if(fork() == 0)
			run_cmd(buf);
		wait();
	}

	printf("sh exit\n");
	return -1;
}

