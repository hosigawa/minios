#include "kernel.h"

struct proc proc_table[MAX_PROC];
struct context *scheduler_context;
struct proc *run_proc;

uint next_pid = 0;

void swtch(struct context **old, struct context *new);

void trapret();

static bool first = true;
void forkret()
{
	if(first) {
		init_fs(1);
		first = false;
	}
}

struct proc *alloc_proc()
{
	struct proc *p = NULL;
	int i = 0;
	for(;i < MAX_PROC; i++) {
		p = proc_table + i;
		if(p->status == UNUSED)
			break;
		p = NULL;
	}
	if(!p)
		return NULL;
	p->kstack = (uint)mem_alloc();
	if(!p->kstack) {
		p->status = UNUSED;
		return NULL;
	}
	p->pid = next_pid++;
	p->status = EMBRYO;
	p->killed = false;
	uint sp = p->kstack + PG_SIZE;
	sp -= sizeof(struct trap_frame);
	p->tf = (struct trap_frame *)sp;
	sp -= 4;
	*(uint *)sp = (uint)trapret;
	sp -= sizeof(struct context);
	p->context = (struct context *)sp;
	p->context->eip = (uint)forkret;
	return p;
}

void scheduler()
{
	while(1) {
		sti();
		int i = 0;
		for(; i < MAX_PROC; i++) {
			struct proc *p = proc_table + i;
			if(p->status == READY) {
				p->status = RUNNING;
				run_proc = p;
				swtch_uvm(p);
				swtch(&scheduler_context, p->context);
				swtch_kvm();
			}
		}
	}
}

void user_init()
{
	extern char _binary__obj_initcode_start[], _binary__obj_initcode_size[];
	struct proc *p = alloc_proc();
	if(!p)
		panic("user_init error\n");
	p->pgdir = set_kvm();
	p->mem_size = PG_SIZE;
	init_uvm(p->pgdir, _binary__obj_initcode_start, (int)_binary__obj_initcode_size);
	
	memset(p->tf, 0, sizeof(*p->tf));
  
	p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  	p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  	p->tf->es = p->tf->ds;
  	p->tf->ss = p->tf->ds;
  	p->tf->eflags = FL_IF;
  	p->tf->esp = PG_SIZE;
  	p->tf->eip = 0;

	p->status = READY;
}

int fork() 
{
	struct proc *p = alloc_proc();
	if(!p)
		return -1;
	p->pgdir = cp_uvm(run_proc->pgdir, run_proc->mem_size);
	if(!p->pgdir) {
		p->status = UNUSED;
		return -1;
	}
	*p->tf = *run_proc->tf;
	p->tf->eax = 0;
	p->parent = run_proc;
	p->mem_size = run_proc->mem_size;
	p->status = READY;

	return p->pid;
}

void sched()
{
	swtch(&run_proc->context, scheduler_context);
}

void yield()
{
	run_proc->status = READY;
	sched();
}

void sleep(void *chan)
{
	run_proc->sleep_chan = chan;
	run_proc->status = SLEPING;
	sched();

	run_proc->sleep_chan = 0;
}

void wakeup(void *chan)
{
	int i = 0;
	for(; i < MAX_PROC; i++) {
		struct proc *p = proc_table + i;
		if(p->status == SLEPING && p->sleep_chan == chan)
			p->status = READY;
	}
}

