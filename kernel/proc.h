#ifndef __PROC_H__
#define __PROC_H__

#define MAX_PROC 64

enum PROC_STATUS {
	UNUSED = 0,
	EMBRYO,
	READY,
	RUNNING,
	SLEPING,
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
	int pid;
	bool killed;
	uint kstack;
	uint mem_size;
	struct trap_frame *tf;
	struct context *context;
	int status;
	pde_t *pgdir;
	struct proc *parent;
	void *sleep_chan;
};

int fork();
void forkret();
struct proc *alloc_proc();
void scheduler();
void sched();
void yield();
void user_init();
void sleep(void *chan);
void wakeup(void *chan);
int exec(char *path, char **argv);
void exit();
int wait();

#endif

