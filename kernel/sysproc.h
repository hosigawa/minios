#ifndef __SYSPROC_H__
#define __SYSPROC_H__

#include "syscall.h"

struct proc_info {
	int pid;
	char name[64];
	uint vsz;
	int stat;
	int ppid;
};

int get_arg_int(int n);
uint get_arg_uint(int n);
char *get_arg_str(int n);
void sys_call();

//syscall functions
int sys_vprintf();
int sys_fork();
int sys_exec();
int sys_exit();
int sys_wait();
int sys_ps();

#endif

