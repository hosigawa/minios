#include "unistd.h"
#include "malloc.h"
#include "stdio.h"

int exec(char *path, char **argv, char **envp)
{
	int ret = execv(path, argv, envp);
	if(ret == -1){
		char full_path[64];
		int envc = 0;
		while(envp[envc]) {
			memset(full_path, 0, 64);
			sprintf(full_path, "%s/%s", envp[envc++], path);
			ret = execv(full_path, argv, envp);
			if(ret == -1)
				continue;
			else
				return ret;
		}		
		return ret;
	}

	return ret;
}

static uint _rand_seek = 0;

static int next_int(char **p)
{
	int sum = 0;
	while(**p != ' ' && **p != '\n') 
		sum = sum * 10 + *((*p)++) - '0';

	while(**p == ' ' || **p == '\n')
		(*p)++;

	return sum;
}

void srand(int seek)
{
	if(seek != 0) {
		_rand_seek = seek;
		return;
	}
	int fd = open("/dev/sys", 0);
	if(fd < 0) {
		_rand_seek = 0;
		return;
	}
	char sysinfo[4096];
	memset(sysinfo, 0, 4096);
	read(fd, sysinfo, 4096);
	char *p = sysinfo;
	next_int(&p);
	next_int(&p);
	next_int(&p);
	next_int(&p);
	next_int(&p);
	next_int(&p);
	_rand_seek = next_int(&p);
	close(fd);
}

int rand()
{
	int q, r, t;
	q = _rand_seek / 127773;
	r = _rand_seek % 127773;
	t = 16807 * r - 2836 * q;
	if(t < 0)
		t += 0x7fffffff;
	return (_rand_seek = t);
}

int localtime(uint *time)
{
	stime(time);
	return 0;
}

extern void __sig_restore(void);
sig_handler signal(int signal, sig_handler handler)
{
	return (sig_handler)ssignal(signal, handler, __sig_restore);
}

