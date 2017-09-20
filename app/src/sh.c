#include "stdio.h"
#include "unistd.h"

struct alias_token {
	char alias[32];
	char token[64];
};

char cwd[DIR_NM_SZ];
char profile[4096];
char env[1024];
char *envp[10];
struct alias_token alias[16];

int getcmd(char *buf, int len)
{
	int ret;
	printf("[root@localhost %s]$ ", cwd);
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
	while(fgets(fd, env, 1024)){
		if(!strncmp(env, "PATH", 4)) {
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

void get_alias()
{
	int i;
	for(i = 0; i < 16; i++) {
		memset(alias + i, 0, sizeof(alias));
	}
	int fd = open("/home/.profile", 0);
	if(fd < 0)
		return;
	char buf[128];
	memset(buf, 0, 128);
	char *p;
	i = 0;
	int j = 0;
	while(fgets(fd, buf, 128)){
		if(!strncmp(buf, "alias", 5)) {
			p = buf + 6;
			j = 0;
			while(*p && *p != '=') {
				alias[i].alias[j++] = *p++;
			}
			j = 0;
			p+=2;
			while(*p && *p != '\"') {
				alias[i].token[j++] = *p++;
			}
			if(i++ == 15)
				break;
		}
		memset(buf, 0, 128);
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
	char *p = buf;
	while(*p) {
		if(*p == ' ' || *p == '\t') {
			*p = 0;
			btk = false;
		}
		else if(!btk) {
			if(seq == 1)
				break;
			argv[seq++] = p;
			btk = true;
		}
		p++;
	}
	btk = false;
	char real_buf[100];
	memset(real_buf, 0, 100);
	for(i = 0; i < 16; i++) {
		if(!strcmp(alias[i].alias, argv[0])) {
			sprintf(real_buf, "%s %s", alias[i].token, p);
			memset(buf, 0, 100);
			strcpy(buf, real_buf);
			p = buf;
			seq = 0;
			break;
		}
	}

	while(*p) {
		if(*p == ' ' || *p == '\t') {
			*p = 0;
			btk = false;
		}
		else if(!btk) {
			argv[seq++] = p;
			btk = true;
		}
		p++;
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
	printf("\n");
	chdir("/home");
	memset(cwd, 0, DIR_NM_SZ);
	int ret = getpwd(cwd, false);
	if(ret < 0){
		printf("pwd ret is %d\n", ret);
		return -1;
	}

	get_envp();
	get_alias();

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
				ret = getpwd(cwd, false);
				if(ret < 0){
					printf("pwd ret is %d\n", ret);
					break;;
				}
			}
			continue;
		}
		else if(strcmp(sargv[0], "pwd") == 0) {
			char full_path[256];
			memset(full_path, 0, 256);
			ret = getpwd(full_path, true);
			if(ret < 0)
				break;
			printf("%s\n", full_path);
			continue;
		}
		if(fork() == 0)
			run_cmd(sargv);
		wait();
	}

	printf("sh exit\n");
	return -1;
}

