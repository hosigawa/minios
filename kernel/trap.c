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
		if((tf->cs & 3) != DPL_USER)
			panic("system call must from userspace\n");
		sys_call();
	} 
	else {
		switch(tf->trapno) {
			case GET_IRQ(IRQ_TIMER):
				timer_proc(tf);
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
					panic("system interrupted, trapno:%d; errno:%d, eip%p\n", tf->trapno, tf->errno, tf->eip);
				printf("pid:%d interrupted, trapno:%d, errno:%d, eip:%p\n", cpu.cur_proc->pid, tf->trapno, tf->errno, tf->eip);
				
				if(tf->trapno == 14)
					kill(cpu.cur_proc->pid, SIG_SEGV);
				kill(cpu.cur_proc->pid, SIG_QUIT);
				break;
		}
	}
	do_signal(tf);
}

