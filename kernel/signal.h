#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#define SIG_KILL 9

typedef void (*sig_handler)(int);
typedef void (*sig_restore)(void);

struct sig_struct {
	sig_handler handler;
	sig_restore restore;
};

struct trap_frame;
struct proc;
void do_signal(struct trap_frame *tf);
void kill(struct proc *p, int singal);

#endif

