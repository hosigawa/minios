#include "kernel.h"

struct proc *run_proc;

int (*syscalls[])(void) = {
	[SYS_print] = sys_print,
	[SYS_fork] = sys_fork,
};

int get_arg_int(int n)
{
	//todo check user mem boundray
	return *(int *)(run_proc->tf->esp + 4 + 4*n);
}

uint get_arg_uint(int n)
{
	//todo check user mem boundray
	return *(uint *)(run_proc->tf->esp + 4 + 4*n);
}

char *get_arg_str(int n)
{
	//todo check user mem boundray
	return *(char **)(run_proc->tf->esp + 4 + 4*n);
}

void sys_call()
{
	int seq = run_proc->tf->eax;
	if(seq <= 0 || seq >= (sizeof(syscalls)/sizeof(syscalls[0]))) {
		err_info("unknow syscall %d\n", seq);
		run_proc->tf->eax = -1;
	}
	run_proc->tf->eax = syscalls[seq]();
}

int sys_print()
{
	//char *arg = (char *)get_arg_uint(0);
	//cprintf("%s", arg);
	return 0;
}

int sys_fork() 
{
	return fork();
}

