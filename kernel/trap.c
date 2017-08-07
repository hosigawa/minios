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
	cli();
	if(tf->trapno == T_SYSCALL) {
		sys_call();
		sti();
		return;
	}

	switch(tf->trapno) {
		case GET_IRQ(IRQ_TIMER):
			tick++;/*
			if(tick % 500 == 0) {
				printf("timer 500, ready to sleep pid:%d\n", run_proc->pid);
				run_proc->status = READY;
				swtch(&run_proc->context, scheduler_context);
			}*/
			break;
		default:
			panic("trap occurd, trapno:%d; errno:%d\n", tf->trapno, tf->errno);
			break;
	}
	sti();
}

