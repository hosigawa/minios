#include "kernel.h"

extern struct cpu cpu;

int (*syscalls[])(void) = {
	[SYS_cprintf] = sys_cprintf,
	[SYS_fork] = sys_fork,
	[SYS_exec] = sys_exec,
	[SYS_exit] = sys_exit,
	[SYS_wait] = sys_wait,
	[SYS_ps] = sys_ps,
};

int get_arg_int(int n)
{
	//todo check user mem boundray
	return *(int *)(cpu.cur_proc->tf->esp + 4 + 4*n);
}

uint get_arg_uint(int n)
{
	//todo check user mem boundray
	return *(uint *)(cpu.cur_proc->tf->esp + 4 + 4*n);
}

char *get_arg_str(int n)
{
	//todo check user mem boundray
	return *(char **)(cpu.cur_proc->tf->esp + 4 + 4*n);
}

void sys_call()
{
	int seq = cpu.cur_proc->tf->eax;
	if(seq <= 0 || seq >= (sizeof(syscalls)/sizeof(syscalls[0]))) {
		err_info("unknow syscall %d\n", seq);
		cpu.cur_proc->tf->eax = -1;
	}
	cpu.cur_proc->tf->eax = syscalls[seq]();
}

int sys_cprintf()
{
	char *fmt = (char *)get_arg_uint(0);
	uint *argp = (uint *)get_arg_uint(1);
	pushcli();
	cprintfarg(fmt, argp);
	popsti();
	return 0;
}

int sys_fork() 
{
	return fork();
}

int sys_exec()
{
	char *path = (char *)get_arg_uint(0);
	char *argv = (char *)get_arg_uint(1);
	return exec(path, &argv);
}

int sys_exit()
{
	exit();
	return 0;
}

int sys_wait()
{
	return wait();
}

int sys_ps()
{
	struct proc_info *pi = (struct proc_info *)get_arg_uint(0);
	int size = get_arg_int(1);
	
	extern struct proc proc_table[];
	struct proc *p;
	int i = 0, j = 0; 
	for(; i < MAX_PROC; i++) {
		p = proc_table + i;
		if(p->stat != UNUSED) {
			pi[j].pid = p->pid;
			memmove(pi[j].name, p->name, PROC_NM_SZ);
			pi[j].vsz = p->vsz;
			pi[j].stat = p->stat;
			pi[j].ppid = p->parent ? p->parent->pid : 0;

			j++;
			if(j >= size)
				break;
		}
	}
	return 0;
}

