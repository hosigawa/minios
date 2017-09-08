#ifndef __TIMER_H__
#define __TIMER_H__

#define TIMER_FREQ      1193182
#define TIMER_DIV(x)    ((TIMER_FREQ+(x)/2)/(x))
#define IO_TIMER1       0x040 
#define TIMER_MODE      (IO_TIMER1 + 3) // timer mode port
#define TIMER_SEL0      0x00    // select counter 0
#define TIMER_RATEGEN   0x04    // mode 2, rate generator
#define TIMER_16BIT     0x30    // r/w counter 16 bits, LSB first

#define CMOS_CTRL 0x70
#define CMOS_DATA 0x71
#define CMOS_SEC 0x0
#define CMOS_MIN 0x2
#define CMOS_HOU 0x4
#define CMOS_DAY 0x7
#define CMOS_MON 0x8
#define CMOS_YEA 0x9
#define BCD_TO_NUM(val) (((val)&0x0f)+((val)>>4)*10)

#define TIME_HZ 1000

void init_timer();

struct trap_frame;
void timer_proc(struct trap_frame *tf);
uint get_systime();

#endif

