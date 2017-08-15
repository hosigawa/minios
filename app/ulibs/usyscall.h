#ifndef __USYSCALL_H__
#define __USYSCALL_H__

void print(char *msg);
int fork();
int exec(char *path, char **argv);
int exit();
int wait();

#endif

