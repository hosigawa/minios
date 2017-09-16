#ifndef __USYSCALL_H__
#define __USYSCALL_H__

#include "type.h"
#include "sysproc.h"
#include "signal.h"
#include "fs.h"

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
int readdir(int fd, struct dirent *de);
int sleep(int ms);
int stime(uint *time);
int signal(int signal, sig_handler handler);
int sigret();
int kill(int sig, int pid);

#endif

