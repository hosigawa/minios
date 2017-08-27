#include "stdio.h"
#include "unistd.h"

char cwd[14];
char profile[4096];
char env[1024];
char *envp[10];

int getcmd(char *buf, int len)
{
	int ret;
	printf("[root@localhost %s]$", cwd);
	memset(buf, 0, len);
	ret = read(stdin, buf, len);
	if(ret <= 0)
		return -1;
	buf[ret - 1] = 0;
	return 0;
}

void get_envp()
{
	int i;
	for(i = 0; i < 10; i++) {
		envp[i] = 0;
	}
	memset(env, 0, 1024);
	int fd = open("/home/.profile", 0);
	if(fd < 0)
		return;
	struct file_stat stat;
	fstat(fd, &stat);
	while(fgets(fd, env, 1024)){
		if(env[0] == 'P' && env[1] == 'A' && env[2] == 'T' && env[3] == 'H') {
			char *p = env;
			int seq = 0;
			while(*p) {
				if(*p == '=') {
					envp[seq] = ++p;
					continue;
				}
				if(*p == ':'){
					*p = 0;
					seq++;
					envp[seq] = ++p;
					continue;
				}
				p++;
			}
			break;
		}
		memset(env, 0, 1024);
	}
	close(fd);
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
	int ret = 0;
	ret = exec(argv[0], argv, envp);
	if(ret == -1) {
		printf("-sh: %s: command not found\n", argv[0]);
	}
	else if(ret == -2) {
		printf("-sh: %s: is a directory\n", argv[0]);
	}
	else if(ret == -3) {
		printf("-sh: %s: cannot execute binary file\n", argv[0]);
	}
	exit();
}

int main(int argc, char **argv) 
{
	printf("init sh...\n");
	chdir("/home");
	memset(cwd, 0, 14);
	int ret = pwd(cwd);
	if(ret < 0){
		printf("pwd ret is %d\n", ret);
		return -1;
	}

	get_envp();

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
			else {
				memset(cwd, 0, 14);
				ret = pwd(cwd);
				if(ret < 0){
					printf("pwd ret is %d\n", ret);
					break;;
				}
			}
			continue;
		}
		if(fork() == 0)
			run_cmd(sargv);
		wait();
	}

	printf("sh exit\n");
	return -1;
}

