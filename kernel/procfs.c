#include "kernel.h"

void init_procfs()
{
}

int procinfo_read(struct inode *ip, char *dst, int len)
{
	extern struct proc proc_table[];
	struct proc *p;
	int i = 0;
	int wd = 0;
	for(; i < MAX_PROC; i++) {
		p = proc_table + i;
		if(p->stat != UNUSED) {
			wd += sprintf(dst + wd, "%d %s %d %d %d\n", p->pid, p->name, p->vend - USER_LINK, p->stat, p->parent ? p->parent->pid : 0);
		}
	}
	return 0;
}

int procinfo_write(struct inode *ip, char *dst, int len)
{
	return 0;
}

int sysinfo_read(struct inode *ip, char *dst, int len)
{
	int wd = 0;
	wd += sprintf(dst + wd, "%d\n", size_of_free_memory() / 1024);
	return 0;
}

int sysinfo_write(struct inode *ip, char *dst, int len)
{
	return 0;
}

