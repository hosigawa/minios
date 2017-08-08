#ifndef __TIMER_H__
#define __TIMER_H__

#define TIMER_FREQ      1193182
#define TIMER_DIV(x)    ((TIMER_FREQ+(x)/2)/(x))
#define IO_TIMER1       0x040 
#define TIMER_MODE      (IO_TIMER1 + 3) // timer mode port
#define TIMER_SEL0      0x00    // select counter 0
#define TIMER_RATEGEN   0x04    // mode 2, rate generator
#define TIMER_16BIT     0x30    // r/w counter 16 bits, LSB first

#define TIME_HZ 100

void init_timer();
void timer_proc();

#endif

