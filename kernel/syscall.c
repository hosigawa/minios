#include "kernel.h"

struct proc *run_proc;

int (*syscalls[])(void) = {
	[SYS_PRINT] = sys_print,
};

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
	cprintf("This is the system call: sys_print\n");
	return 0;
}

