#include "kernel.h"

void init_timer()
{
	outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
	outb(IO_TIMER1, TIMER_DIV(TIME_HZ) % 256);
	outb(IO_TIMER1, TIMER_DIV(TIME_HZ) / 256);
	enable_pic(IRQ_TIMER);
}

