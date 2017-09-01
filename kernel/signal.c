#include "kernel.h"

extern struct CPU cpu;

void do_signal(struct trap_frame *tf)
{	
	if(!cpu.cur_proc || cpu.cur_proc->signal == 0 || (tf->cs & 3) != DPL_USER)
		return;
	sig_handler handler = cpu.cur_proc->sig_handlers[cpu.cur_proc->signal];
	if(handler) {
		int old_eip = tf->eip;
		tf->esp -= old_eip;
		return;
	}
	printf("pid:%d abort, trapno:%d, errno:%d, eip:%p\n", cpu.cur_proc->pid, tf->trapno, tf->errno, tf->eip);
	switch(cpu.cur_proc->signal) {
		case SIG_KILL:
			exit();
			break;
	}
	cpu.cur_proc->signal = 0;
}

void kill(struct proc *p, int signal)
{
	if(!p)
		return;
	p->signal = signal;
}

