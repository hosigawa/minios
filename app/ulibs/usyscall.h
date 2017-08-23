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

#define DIR_NM_SZ 14
struct dirent {
	short inum;
	char name[DIR_NM_SZ];
};

#define T_DIR 1
#define T_FILE 2
#define T_DEV 3
struct file_stat {
	short type;           // File type
  	short nlink;          // Number of links to inode in file system
  	uint size;            // Size of file (bytes)
};

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
int fstat(int fd, struct file_stat *st);
int ps(struct proc_info *pi, int len);
int mkdir(char *path);
int chdir(char *path);
int unlink(char *path);
int pwd(char *wd);

#endif

