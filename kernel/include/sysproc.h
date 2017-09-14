#ifndef __SYSPROC_H__
#define __SYSPROC_H__

#include "syscall.h"

struct file_stat {
	short type;           // File type
  	short nlink;          // Number of links to inode in file system
  	uint size;            // Size of file (bytes)
	uint ctime;
	uint mtime;
	uint atime;
};

int get_arg_int(int n);
uint get_arg_uint(int n);
char *get_arg_str(int n);
void sys_call();

//syscall functions
int sys_fork();
int sys_execv();
int sys_exit();
int sys_wait();
int sys_open();
int sys_close();
int sys_dup();
int sys_mknod();
int sys_read();
int sys_write();
int sys_fstat();
int sys_pwd();
int sys_mkdir();
int sys_chdir();
int sys_unlink();
int sys_sbrk();
int sys_sleep();
int sys_stime();
int sys_signal();
int sys_sigret();
int sys_kill();
int sys_readdir();

#endif

