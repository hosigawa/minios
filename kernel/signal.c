#include "kernel.h"

extern struct CPU cpu;

void do_signal(struct trap_frame *tf)
{	
	if(!cpu.cur_proc || cpu.cur_proc->signal == 0 || (tf->cs & 3) != DPL_USER)
		return;
	sig_handler handler = cpu.cur_proc->sig_handlers[cpu.cur_proc->signal - 1].handler;
	sig_restore restore = cpu.cur_proc->sig_handlers[cpu.cur_proc->signal - 1].restore;
	if(handler) {
		uint old_eip = tf->eip;
		uint *old_esp = (uint *)tf->esp;
		tf->eip = (uint)handler;
		*(old_esp - 1) = old_eip;
		*(old_esp - 2) = tf->eflags;
		*(old_esp - 3) = tf->eax;
		*(old_esp - 4) = tf->ebx;
		*(old_esp - 5) = tf->ecx;
		*(old_esp - 6) = tf->edx;
		*(old_esp - 7) = cpu.cur_proc->signal;
		*(old_esp - 8) = (uint)restore;
		tf->esp -= 32;
		cpu.cur_proc->signal = 0;
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

