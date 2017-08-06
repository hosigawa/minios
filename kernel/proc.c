#include "kernel.h"

struct proc proc_table[MAX_PROC];
struct context *scheduler_context;

void swtch(struct context **old, struct context *new);

void scheduler()
{
	sti();
	int i = 0;
	for(; i < MAX_PROC; i++) {
		struct proc *p = proc_table + i;
		if(p->status == READY) {
			p->status = RUNNING;
			swtch_uvm(p);
			swtch(&scheduler_context, p->context);
			swtch_kvm();
		}
	}
}

