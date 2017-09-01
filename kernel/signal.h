#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#define SIG_KILL 9

struct trap_frame;
struct proc;
void do_signal(struct trap_frame *tf);
void kill(struct proc *p, int singal);

#endif

