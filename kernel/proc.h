#ifndef __PROC_H__
#define __PROC_H__

#define MAX_PROC 64

enum PROC_STATUS {
	UNUSED = 0,
	READY,
	RUNNING,
	SLEPPING,
};

struct context {
	uint edi;
	uint esi;
	uint ebx;
	uint ebp;
	uint eip;
};

struct proc {
	int pid;
	uint kstack;
	struct context *context;
	int status;
	pde_t *pgdir;
};

void scheduler();

#endif

