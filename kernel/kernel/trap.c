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

void do_trap(struct trap_frame *tf)
{
	if(!cpu.cur_proc || (tf->cs & 3) == DPL_KERN)
		panic("system interrupted, trapno:%d; errno:%d, eip%p\n", tf->trapno, tf->errno, tf->eip);

	if(tf->trapno == 14) {
		printf("pid:%d lost page %p, eip:%p\n", cpu.cur_proc->pid, read_cr2(), tf->eip);
		kill(cpu.cur_proc->pid, SIG_KILL);
	}

	printf("userspace interrupted, pid:%d, trapno:%d, errno:%d, eip:%p\n", cpu.cur_proc->pid, tf->trapno, tf->errno, tf->eip);
				
	kill(cpu.cur_proc->pid, SIG_KILL);
}

void trap(struct trap_frame *tf) 
{
	if(tf->trapno == T_SYSCALL) {
		if((tf->cs & 3) != DPL_USER)
			panic("system call must came from userspace\n");
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
				do_trap(tf);
				break;
		}
	}
	do_signal(tf);
}

