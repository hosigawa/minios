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
			wd += sprintf(dst + wd, "%d %d %d %d %d %s\n", 
									p->pid, 
									p->parent ? p->parent->pid : 0,
									p->vend - USER_LINK,
									p->stat,
									p->ticks,
									p->name
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

	extern uint user_ticks, ticks;
	wd += sprintf(dst, "%d %d %d %d %d %d\n", 
						PHYSICAL_END, 
						size_of_free_memory(),
						sb.ninodes,
						inodes,
						user_ticks,
						ticks
				);
	return wd;
}

int sysinfo_write(struct inode *ip, char *dst, int off, int len)
{
	return 0;
}

