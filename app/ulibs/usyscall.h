#ifndef __USYSCALL_H__
#define __USYSCALL_H__

#include "type.h"

struct proc_info {
	int pid;
	char name[64];
	uint vsz;
	int stat;
	int ppid;
};

int vprintf(char *fmt, uint *argp);
int fork();
int exec(char *path, char **argv);
int exit();
int wait();
int ps(struct proc_info *pi, int len);

#endif

