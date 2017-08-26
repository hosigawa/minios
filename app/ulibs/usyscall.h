#ifndef __USYSCALL_H__
#define __USYSCALL_H__

#include "type.h"

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

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

int fork();
int execv(char *path, char **argv, char **envp);
int exit() __attribute__((noreturn));
int wait();
int open(char *path, int mode);
int close(int fd);
int dup(int fd);
int mknod(char *path, int major, int minor);
int read(int fd, char *dst, int len);
int write(int fd, char *src, int len);
int fstat(int fd, struct file_stat *st);
int mkdir(char *path);
int chdir(char *path);
int unlink(char *path);
int sbrk(int sz);
int pwd(char *wd);
int sleep(int ms);

#endif

