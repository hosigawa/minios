#include "libc.h"

int getcmd(char *buf, int len)
{
	int ret;
	printf("$");
	memset(buf, 0, len);
	ret = read(stdin, buf, len);
	if(ret <= 0)
		return -1;
	buf[ret - 1] = 0;
	return 0;
}

int run_cmd(char *buff)
{
	char *argv[] = {0};
	if(*buff == 0 || *buff == '\n')
		exit();
	exec(buff, argv);
	printf("-sh: %s: command not found\n", buff);
	exit();
}

int main() 
{
	printf("init sh...\n");
	//char *argv[] = {"ps", 0};
	char buf[100];

	while(getcmd(buf, sizeof(buf)) >= 0) {	
		if(fork() == 0)
			run_cmd(buf);
		wait();
	}

	printf("sh exit\n");
	return -1;
}

