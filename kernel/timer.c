#include "kernel.h"

extern struct CPU cpu;

uint boot_time = 0;
uint user_ticks = 0;
uint kern_ticks = 0;
uint ticks = 0;

void init_timer()
{
	outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
	outb(IO_TIMER1, TIMER_DIV(TIME_HZ) % 256);
	outb(IO_TIMER1, TIMER_DIV(TIME_HZ) / 256);
	enable_pic(IRQ_TIMER);
}

void timer_proc(struct trap_frame *tf)
{
	if(!boot_time)
		init_localtime();

	ticks++;

	wakeup_on(timer_proc);
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

int init_localtime()
{
	int year, mon, day, h, m, s;
	outb(CMOS_CTRL, CMOS_SEC);
	s = BCD_TO_NUM(inb(CMOS_DATA));
	
	outb(CMOS_CTRL, CMOS_MIN);
	m = BCD_TO_NUM(inb(CMOS_DATA));
	
	outb(CMOS_CTRL, CMOS_HOU);
	h = BCD_TO_NUM(inb(CMOS_DATA));

	outb(CMOS_CTRL, CMOS_DAY);
	day = BCD_TO_NUM(inb(CMOS_DATA));

	outb(CMOS_CTRL, CMOS_MON);
	mon = BCD_TO_NUM(inb(CMOS_DATA));

	outb(CMOS_CTRL, CMOS_YEA);
	year = 2000 + BCD_TO_NUM(inb(CMOS_DATA));

	int ys = 0;
	ys += ((year - 1970) / 4 ) * (365 * 3 + 366);
	int y_mod = (year - 1970) % 4;
	switch(y_mod) {
		case 1:
			ys += 365;
			break;
		case 2:
			ys += 365 * 2;
		break;
		case 3:
			ys += 365 * 2 + 366;
		break;
	}

	static int mon1[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	static int mon2[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int ms = 0;
	int i;
	if(year % 400 == 0 || ((year % 4 == 0) && (year % 100 != 0))) {
		for(i = 0; i < mon - 1; i++)
			ms += mon2[i];
	}
	else {
		for(i = 0; i < mon - 1; i++)
			ms += mon1[i];
	}

	int ds = 0;
	ds += day - 1;

	boot_time = ys * 3600 * 24 + ms * 3600 * 24 + ds * 3600 * 24 + h * 3600 + m * 60 + s;
	return boot_time;
}

