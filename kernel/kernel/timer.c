#include "kernel.h"

extern struct CPU cpu;

uint boot_time = 0;
uint user_ticks = 0;
uint kern_ticks = 0;
uint ticks = 0;

static unixstamp_t init_localtime()
{
	struct time_v tm;

	outb(CMOS_CTRL, CMOS_SEC);
	tm.s = BCD_TO_NUM(inb(CMOS_DATA));
	
	outb(CMOS_CTRL, CMOS_MIN);
	tm.m = BCD_TO_NUM(inb(CMOS_DATA));
	
	outb(CMOS_CTRL, CMOS_HOU);
	tm.h = BCD_TO_NUM(inb(CMOS_DATA));

	outb(CMOS_CTRL, CMOS_DAY);
	tm.day = BCD_TO_NUM(inb(CMOS_DATA));

	outb(CMOS_CTRL, CMOS_MON);
	tm.mon = BCD_TO_NUM(inb(CMOS_DATA));

	outb(CMOS_CTRL, CMOS_YEA);
	tm.year = 2000 + BCD_TO_NUM(inb(CMOS_DATA));

	return time2unixstamp(&tm);
}

void init_timer()
{
	outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
	outb(IO_TIMER1, TIMER_DIV(TIME_HZ) % 256);
	outb(IO_TIMER1, TIMER_DIV(TIME_HZ) / 256);
	enable_pic(IRQ_TIMER);
	boot_time = init_localtime();
}

void timer_proc(struct trap_frame *tf)
{
	static bool begin_tick = false;
	if(!begin_tick) {
		boot_time = init_localtime();
		begin_tick = true;
	}

	ticks++;

	wakeup_on(&ticks);
	if(cpu.cur_proc ) {
		if((tf->cs & 3) == DPL_USER)
			user_ticks++;
		else
			kern_ticks++;

		if(cpu.cur_proc->stat == RUNNING) {
			cpu.cur_proc->ticks++;
			if(cpu.cur_proc->count > 0)
				cpu.cur_proc->count--;
			else if((tf->cs & 3) == DPL_USER)
				yield();
		}
	}
}

uint get_systime()
{
	return boot_time + ticks / TIME_HZ;
}

