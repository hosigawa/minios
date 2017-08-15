#include "kernel.h"

extern struct cpu cpu;

uint tick = 0;

void init_timer()
{
	outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
	outb(IO_TIMER1, TIMER_DIV(TIME_HZ) % 256);
	outb(IO_TIMER1, TIMER_DIV(TIME_HZ) / 256);
	enable_pic(IRQ_TIMER);
}

void timer_proc()
{
	tick++;
	if(tick >= 1) {
		tick = 0;
		if(cpu.cur_proc && cpu.cur_proc->status == RUNNING) {
			yield();
		}
	}
}

