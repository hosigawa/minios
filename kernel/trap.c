#include "kernel.h"

struct gate_desc idt[256];
extern uint vectors[];

uint tick = 0;

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
	switch(tf->trapno) {
		case GET_IRQ(IRQ_TIMER):
			//printf("tick is %d\n", tick++);
			break;
		default:
			panic("trap occurd, trapno:%d\n", tf->trapno);
			break;
	}
}

