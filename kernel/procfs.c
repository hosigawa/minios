#include "kernel.h"

void init_procfs()
{
}

int procinfo_read(struct inode *ip, char *dst, int off, int len)
{
	extern struct proc proc_table[];
	struct proc *p;
	int i = 0;
	int wd = 0;

	for(; i < MAX_PROC; i++) {
		p = proc_table + i;
		if(p->stat != UNUSED) {
			wd += sprintf(dst + wd, "%d %d %d %d %d %s %d\n", 
									p->pid, 
									p->parent ? p->parent->pid : 0,
									p->vend - USER_LINK,
									p->stat,
									p->ticks,
									p->name,
									p->priority
						);
		}
		if(wd > 3800)
			break;
	}
	return wd;
}

int procinfo_write(struct inode *ip, char *dst, int off, int len)
{
	return 0;
}

int sysinfo_read(struct inode *ip, char *dst, int off, int len)
{
	int wd = 0;
	extern uint _version;
	extern uint user_ticks, kern_ticks, ticks;
	wd += sprintf(dst, "%d %d %d %d %d %d\n", 
						_version,
						PHYSICAL_END, 
						size_of_free_memory(),
						user_ticks,
						kern_ticks,
						ticks
				);
	return wd;
}

int sysinfo_write(struct inode *ip, char *dst, int off, int len)
{
	return 0;
}

