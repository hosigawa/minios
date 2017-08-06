#ifndef __PROC_H__
#define __PROC_H__

#define MAX_PROC 64

enum PROC_STATUS {
	UNUSED = 0,
	EMBRYO,
	READY,
	RUNNING,
	SLEPING,
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
	uint pid;
	uint kstack;
	struct trap_frame *tf;
	struct context *context;
	int status;
	pde_t *pgdir;
};

void forkret();
struct proc *alloc_proc();
void scheduler();
void user_init();

#endif

