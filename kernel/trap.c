#include "kernel.h"

struct gate_desc idt[256];
extern uint vectors[];

uint tick = 0;

void swtch(struct context **old, struct context *new);
extern struct proc *run_proc;
extern struct context *scheduler_context;

void init_idt() 
{
	int i = 0;
	for(; i < 256; i++) {
		SET_GATE(idt[i], 0, SEG_KCODE << 3, vectors[i], 0);
	}
	SET_GATE(idt[T_SYSCALL], 1, SEG_KCODE << 3, vectors[T_SYSCALL], DPL_USER);
	lidt(idt, sizeof(idt));
}

void trap(struct trap_frame *tf) 
{
	if(tf->trapno == T_SYSCALL) {
		cli();
		sys_call();
		return;
	}

	switch(tf->trapno) {
		case GET_IRQ(IRQ_TIMER):
			tick++;
			if(tick >= 1) {
				tick = 0;
				if(run_proc && run_proc->status == RUNNING) {
					run_proc->status = READY;
					swtch(&run_proc->context, scheduler_context);
				}
			}
			break;
		default:
			panic("trap occurd, trapno:%d; errno:%d\n", tf->trapno, tf->errno);
			break;
	}
}

