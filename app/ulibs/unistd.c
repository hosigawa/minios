#include "unistd.h"
#include "malloc.h"
#include "usyscall.h"
#include "stdio.h"

int exec(char *path, char **argv)
{
	char *envp[10];
	int i;
	for(i = 0; i < 10; i++) {
		envp[i] = 0;
	}
	int fd = open("/home/sh_profile", 0);
	struct file_stat stat;
	if(fd > 0) {
		fstat(fd, &stat);
		char profile[stat.size];

		read(fd, profile, stat.size);

		char *p = profile;
		int seq = 0;
		bool bpath = false;
		while(*p && stat.size-- > 0) {
			if(!bpath && p[0] == 'P' && p[1] == 'A' && p[2] == 'T' && p[3] == 'H') {
				p += 4;
				bpath = true;
				continue;
			}
			if(!bpath) {
				p++;
				continue;
			}
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
			if(*p == '\n') {
				*p = 0;
				bpath = false;
				break;
			}
			p++;
		}
		close(fd);

		return execv(path, argv, envp);
	}

	return execv(path, argv, envp);
}

