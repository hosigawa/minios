#ifndef __UNISTD_H__
#define __UNISTD_H__

#include "libc.h"

int exec(char *path, char **argv, char **envp);
void srand(int seek);
int rand();
int localtime(uint *time);

#endif

