#ifndef __UNISTD_H__
#define __UNISTD_H__

#include "libc.h"
#include "usyscall.h"

int exec(char *path, char **argv, char **envp);
void srand(int seek);
int rand();
int localtime(uint *time);
sig_handler signal(int signal, sig_handler handler);

#endif

