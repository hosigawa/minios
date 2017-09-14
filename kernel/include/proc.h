#ifndef __PROC_H__
#define __PROC_H__

#include "fs.h"
#include "signal.h"
#include "type.h"

#define MAX_PROC 64
#define PROC_NM_SZ 64
#define NOFILE 16

enum PROC_STATUS {
	UNUSED = 0,
	EMBRYO,
	RUNNING,
	SLEEPING,
	ZOMBIE,
};

struct context {
	uint edi;
	uint esi;
	uint ebx;
	uint ebp;
	uint eip;
};

struct trap_frame;

struct proc {
	struct list_t list_head;
	int pid;
	char name[PROC_NM_SZ];
	int killed;
	uint kstack;
	uint vend;
	struct trap_frame *tf;
	struct context *context;
	int stat;
	pde_t *pgdir;
	struct file *ofile[NOFILE];
	struct inode *cwd;
	struct proc *parent;
	void *sleep_chan;
	int signal;
	struct sig_struct sig_handlers[32];
	int priority;
	uint count;
	uint ticks;
};

int fork();
void forkret();
struct proc *alloc_proc();
void scheduler() __attribute__((noreturn));
void sched();
void yield();
void user_init();
void sleep_on(void *chan);
void wakeup_on(void *chan);
void sleep();
void wakeup(struct proc *p);
int execv(char *path, char *argv[], char *envp[]);
void exit();
int wait();
struct proc *get_proc(int pid);

#endif

