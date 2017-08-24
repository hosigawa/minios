#include "kernel.h"

struct gate_desc idt[256];
extern uint vectors[];
extern struct CPU cpu;

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
		sys_call();
		return;
	}

	switch(tf->trapno) {
		case GET_IRQ(IRQ_TIMER):
			timer_proc();
			break;
		case GET_IRQ(IRQ_IDE):
			ide_proc();
			break;
		case GET_IRQ(IRQ_KBD):
			kbd_proc();
			break;
		case GET_IRQ(IRQ_COM1):
			uart_proc();
			break;
		default:
			if(!cpu.cur_proc || (tf->cs & 3) == DPL_KERN)
				panic("system trap occurd, trapno:%d; errno:%d\n", tf->trapno, tf->errno);
			
			printf("pid:%d abort, trapno:%d, errno:%d, eip:%p\n", cpu.cur_proc->pid, tf->trapno, tf->errno, tf->eip);
			cpu.cur_proc->killed = 1;
			break;
	}

	if(cpu.cur_proc && cpu.cur_proc->killed == 1 && (tf->cs & 3) == DPL_USER)
		exit();
}

