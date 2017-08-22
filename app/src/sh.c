#include "libc.h"

char wd[14];
char *env = "/bin/";

int getcmd(char *buf, int len)
{
	int ret;
	memset(wd, 0, 14);
	ret = pwd(wd);
	if(ret < 0){
		printf("pwd ret is %d\n", ret);
		return -1;
	}
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
	int i;
	for(i = 0; i < 10; i++)
	{
		argv[i] = 0;
	}
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

int run_cmd(char **argv)
{
	char real[64] = {0};
	if(exec(argv[0], argv) == -2) {
		printf("-sh: %s: is a directory\n", argv[0]);
		exit();
	}
	memmove(real, env, strlen(env));
	memmove(real+strlen(env), argv[0], strlen(argv[0])+1);
	exec(real, argv);
	printf("-sh: %s: command not found\n", argv[0]);
	exit();
}

int main(int argc, char **argv) 
{
	printf("init sh...\n");
	chdir("/home");
	char buf[100];
	char *sargv[10];

	while(getcmd(buf, sizeof(buf)) >= 0) {
		if(*buf == 0 || *buf == '\n')
			continue;
		get_token(sargv, buf);
		if(strcmp(sargv[0], "cd") == 0){
			if(!sargv[1])
				continue;
			if(chdir(sargv[1]) < 0)
				printf("-sh: cd: %s: No such file or directory\n", sargv[1]);
			continue;
		}
		if(fork() == 0)
			run_cmd(sargv);
		wait();
	}

	printf("sh exit\n");
	return -1;
}

