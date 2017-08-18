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

void print(char *fmt);
int fork();
int exec(char *path, char **argv);
int exit() __attribute__((noreturn));
int wait();
int open(char *path, int mode);
int close(int fd);
int dup(int fd);
int mknod(char *path, int major, int minor);
int read(int fd, char *dst, int len);
int write(int fd, char *src, int len);
int ps(struct proc_info *pi, int len);

#endif

