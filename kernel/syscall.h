#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "mmu.h"

int get_arg_int(int n);
uint get_arg_uint(int n);
char *get_arg_str(int n);
void sys_call();

//syscall functions
int sys_print();
int sys_fork();

#endif

