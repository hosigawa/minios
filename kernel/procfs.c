#include "kernel.h"

void init_procfs()
{
}

static char *STATUS[] = {
	"UNUSED  ", 
	"EMBRYO  ",
	"READY   ",
	"RUNNING ",
	"SLEEPING",
	"ZOMBIE  ",
};

int procinfo_read(struct inode *ip, char *dst, int len)
{
	extern struct proc proc_table[];
	struct proc *p;
	int i = 0;
	int wd = 0;
	for(; i < MAX_PROC; i++) {
		p = proc_table + i;
		if(p->stat != UNUSED) {
			wd += sprintf(dst + wd, "%4d %4d %5d  %s  %s\n", p->pid, p->parent ? p->parent->pid : 0, (p->vend - USER_LINK) / 1024, STATUS[p->stat], p->name);
		}
		if(wd > 3072)
			break;
	}
	return wd;
}

int procinfo_write(struct inode *ip, char *dst, int len)
{
	return 0;
}

int sysinfo_read(struct inode *ip, char *dst, int len)
{
	int wd = 0;
	wd += sprintf(dst + wd, "Memory Total %d kB\n", PHYSICAL_END / 1024);
	wd += sprintf(dst + wd, "Memory Free  %d kB\n", size_of_free_memory() / 1024);

	int off, i;
	struct block_buf *buf;
	struct dinode *dp;
	int inodes = 0;
	extern struct super_block sb;
	for(off = 0; off < sb.ninodes; off += IPER) {
		buf = bread(1, IBLOCK(off));
		for(i = 0; i < IPER; i++) {
			if(off == 0 && i == 0)
				continue;
			dp = (struct dinode *)buf->data + i;
			if(dp->type == 0) {
				inodes++;
			}
		}
		brelse(buf);
	}
	wd += sprintf(dst + wd, "Inode Total %d\n", sb.ninodes);
	wd += sprintf(dst + wd, "Inode Free  %d\n", inodes);
	return wd;
}

int sysinfo_write(struct inode *ip, char *dst, int len)
{
	return 0;
}

