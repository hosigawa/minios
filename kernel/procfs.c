#include "kernel.h"

void init_procfs()
{
}

static char *STATUS[] = {
	"U ", 
	"E ",
	"R-",
	"R+",
	"S ",
	"Z ",
};

int procinfo_read(struct inode *ip, char *dst, int off, int len)
{
	extern struct proc proc_table[];
	struct proc *p;
	int i = 0;
	int wd = 0;

	int pid;
	int ppid;
	uint vsz;
	char *st;
	uint min,sec,ms;
	char *nm;
	for(; i < MAX_PROC; i++) {
		p = proc_table + i;
		if(p->stat != UNUSED) {
			pid = p->pid;
			ppid = p->parent ? p->parent->pid : 0;
			vsz = (p->vend - USER_LINK) / 1024;
			st = STATUS[p->stat];
			min = (p->ticks / 100) / 60;
			sec = (p->ticks / 100) % 60;
			ms = (p->ticks % 100) / 10;
			nm = p->name;

			wd += sprintf(dst + wd, "%4d  %4d  %5d  %s  %4d:%02d.%d  %s\n", pid, ppid, vsz, st, min, sec, ms, nm);
		}
		if(wd > 3072)
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
	wd += sprintf(dst + wd, "Memory Total : %d kB\n", PHYSICAL_END / 1024);
	wd += sprintf(dst + wd, "Memory Free  : %d kB\n", size_of_free_memory() / 1024);

	int offset, i;
	struct block_buf *buf;
	struct dinode *dp;
	int inodes = 0;
	extern struct super_block sb;
	for(offset = 0; offset < sb.ninodes; offset += IPER) {
		buf = bread(1, IBLOCK(offset));
		for(i = 0; i < IPER; i++) {
			if(offset == 0 && i == 0)
				continue;
			dp = (struct dinode *)buf->data + i;
			if(dp->type == 0) {
				inodes++;
			}
		}
		brelse(buf);
	}
	wd += sprintf(dst + wd, "Inode Total  : %d\n", sb.ninodes);
	wd += sprintf(dst + wd, "Inode Free   : %d\n", inodes);
	return wd;
}

int sysinfo_write(struct inode *ip, char *dst, int off, int len)
{
	return 0;
}

