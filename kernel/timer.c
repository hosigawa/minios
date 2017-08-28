#include "kernel.h"

extern struct CPU cpu;

uint user_ticks = 0;
uint ticks = 0;

void init_timer()
{
	outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
	outb(IO_TIMER1, TIMER_DIV(TIME_HZ) % 256);
	outb(IO_TIMER1, TIMER_DIV(TIME_HZ) / 256);
	enable_pic(IRQ_TIMER);
}

void timer_proc()
{
	wakeup(timer_proc);
	if(cpu.cur_proc ) {
		if((cpu.cur_proc->tf->cs & 3) == DPL_USER)
			user_ticks++;

		if(cpu.cur_proc->stat == RUNNING)
			cpu.cur_proc->ticks++;
	}

	if(!(ticks++ % 5)) {
		if(cpu.cur_proc && cpu.cur_proc->stat == RUNNING) {
			yield();
		}
	}
}

