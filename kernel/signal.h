#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#define SIG_HUP 1
#define SIG_INT 2
#define SIG_QUIT 3
#define SIG_ILL 4
#define SIG_TRAP 5
#define SIG_ABRT 6
#define SIG_BUS 7
#define SIG_FPE 8
#define SIG_KILL 9
#define SIG_USR1 10
#define SIG_SEGV 11
#define SIG_USR2 12
#define SIG_PIPE 13
#define SIG_ALRM 14
#define SIG_TERM 15
#define SIG_CHLD 17
#define SIG_CONT 18
#define SIG_STOP 19
#define SIG_TSTP 20
#define SIG_TIN 21
#define SIG_TTOU 22
#define SIG_URG 23
#define SIG_XCPU 24
#define SIG_XFSZ 25

#define SIG_IGN 1

#define SIGNAL(x) (1 << ((x) - 1))

typedef void (*sig_handler)(int);
typedef void (*sig_restore)(void);

struct sig_struct {
	sig_handler handler;
};

struct trap_frame;
struct proc;
void do_signal(struct trap_frame *tf);
int kill(int pid, int signal);

#endif

