#include "kernel.h"

struct proc proc_table[MAX_PROC];
extern struct cpu cpu;

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
	else {
		popsti();
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
	p->pid = ++next_pid;
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
	struct proc *p;
	int i;
	while(1) {
		sti();
		//pushcli();
		for(i = 0; i < MAX_PROC; i++) {
			p = proc_table + i;
			if(p->status == READY) {
				p->status = RUNNING;
				cpu.cur_proc = p;
				swtch_uvm(p);
				swtch(&cpu.context, p->context);
				swtch_kvm();
				cpu.cur_proc = NULL;
			}
		}
		//popsti();
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
	p->pgdir = cp_uvm(cpu.cur_proc->pgdir, cpu.cur_proc->mem_size);
	if(!p->pgdir) {
		p->status = UNUSED;
		return -1;
	}
	*p->tf = *cpu.cur_proc->tf;
	p->tf->eax = 0;
	p->parent = cpu.cur_proc;
	p->mem_size = cpu.cur_proc->mem_size;
	p->status = READY;

	return p->pid;
}

void sched()
{
	if(cpu.ncli != 1)
		panic("pid:%d sched ncli:%d\n", cpu.cur_proc->pid, cpu.ncli);
	if(read_eflags() & FL_IF)
		panic("pid:%d sched FL_IF\n", cpu.cur_proc->pid);
	swtch(&cpu.cur_proc->context, cpu.context);
}

void yield()
{
	pushcli();
	cpu.cur_proc->status = READY;
	sched();
	popsti();
}

void sleep(void *chan)
{
	pushcli();
	cpu.cur_proc->sleep_chan = chan;
	cpu.cur_proc->status = SLEPING;
	sched();

	cpu.cur_proc->sleep_chan = 0;
	popsti();
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

int exec(char *path, char **argv)
{
	struct inode *ip = namei(path);
	if(!ip) {
		err_info("%s: no such file or directory\n", path);
		return -1;
	}

	load_inode(ip);
	struct elfhdr elf;
	readi(ip, (char *)&elf, 0, sizeof(struct elfhdr));
	if(elf.magic != ELF_MAGIC) {
		err_info("elf magic error: %p\n", elf.magic);
		irelese(ip);
		return -1;
	}

	pde_t *pdir = set_kvm();
	if(!pdir)
		panic("exec set_kvm\n");

	struct proghdr ph;
	uint size = 0;
	uint off = elf.phoff;
	int num = 0;
	while(num++ < elf.phnum) {
		readi(ip, (char *)&ph, off, sizeof(struct proghdr));
		off += sizeof(struct proghdr);
		if(ph.type != ELF_PROG_LOAD)
			continue;
		size = resize_uvm(pdir, size, ph.va + ph.memsz);
		load_uvm(pdir, ip, (char *)ph.va, ph.offset, ph.filesz);
	}
	irelese(ip);
	size = PG_ROUNDUP(size);
	size = resize_uvm(pdir, size, size + 2 * PG_SIZE);
	
	pde_t *old = cpu.cur_proc->pgdir;
	cpu.cur_proc->pgdir = pdir;
	cpu.cur_proc->mem_size = size;
	cpu.cur_proc->tf->esp = size;
	cpu.cur_proc->tf->eip = elf.entry;

	swtch_uvm(cpu.cur_proc);

	free_uvm(old);
	return 0;
}

void exit() 
{

}

int wait()
{
	int pid;
	struct proc *p;
	int i;
	while(1) {
		for(i = 0; i < MAX_PROC; i++) {
			p = proc_table + i;
			if(p->status == ZOMBIE && p->parent == cpu.cur_proc) {
				pid = p->pid;
				mem_free((char *)p->kstack);
				p->kstack = NULL;
				free_uvm(p->pgdir);
				p->pid = 0;
				p->parent = NULL;
				p->status = UNUSED;
				return pid;
			}
		}
		sleep(cpu.cur_proc);
	}
	return 0;
}
