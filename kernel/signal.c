#include "kernel.h"

extern struct CPU cpu;

void do_signal(struct trap_frame *tf)
{	
	static char retcode[8] = {0x58, 0xb8, 0x14, 0x00, 0x00, 0x00, 0xcd, 0x40};
	if(!cpu.cur_proc || !cpu.cur_proc->signal || (tf->cs & 3) != DPL_USER)
		return;
	int signal = bsf(cpu.cur_proc->signal) + 1;
	cpu.cur_proc->signal &= ~SIGNAL(signal);

	sig_handler handler = cpu.cur_proc->sig_handlers[signal - 1].handler;
	if(handler) {
		if((int)handler == SIG_IGN)
			return;

		uint *old_esp = (uint *)tf->esp;
		memcpy(old_esp - 2, retcode, 8);
		*(old_esp - 3) = tf->eip;
		*(old_esp - 4) = tf->eflags;
		*(old_esp - 5) = tf->eax;
		*(old_esp - 6) = tf->ecx;
		*(old_esp - 7) = tf->edx;
		*(old_esp - 8) = signal;
		*(old_esp - 9) = (uint)(old_esp - 2);
		tf->esp -= 36;
		tf->eip = (uint)handler;
		return;
	}
	switch(signal) {
		case SIG_CHLD:
			break;
		default:
			exit();
			break;
	}
}

int kill(int pid, int signal)
{
	struct proc *p = get_proc(pid);
	if(!p)
		return -1;
	p->signal |= SIGNAL(signal);
	if(p->stat == SLEEPING) {
		wakeup(p);
	}
	return 0;
}

