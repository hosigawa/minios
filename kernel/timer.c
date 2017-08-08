#include "kernel.h"

extern struct proc *run_proc;
extern struct context *scheduler_context;

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
		if(run_proc && run_proc->status == RUNNING) {
			yield();
		}
	}
}

