#include "kernel.h"

int proc_proc_readdir(struct file *f, struct dirent *de)
{
	struct inode *dp = f->ip;
	int pid = dp->inum >> 16;
	if(dp->type != T_DIR)
		return -1;
	switch(f->off) {
		case 0:
			de->inum = dp->inum;
			sprintf(de->name, ".");
			break;
		case 1:
			de->inum = dp->sb->root->inum;
			sprintf(de->name, "..");
			break;
		case 2:
			de->inum = pid << 16 | 2;
			sprintf(de->name, "stat");
			break;
		case 3:
			de->inum = pid << 16 | 3;
			sprintf(de->name, "cmdline");
			break;
		default:
			return -1;
	}
	f->off++;
	return 0;
}

struct inode *proc_proc_lookup(struct inode *dp, char *name, int *off)
{
	int pid = dp->inum >> 16;
	int low = dp->inum & 0xffff;
	if(low != 1)
		return NULL;
	if(!strcmp(name, ".")) {
		return idup(dp);
	}
	else if(!strcmp(name, "..")) {
		return idup(dp->sb->root);
	}
	else if(!strcmp(name, "stat")) {
		return iget(dp->sb, pid << 16 | 2);
	}
	else if(!strcmp(name, "cmdline")) {
		return iget(dp->sb, pid << 16 | 3);
	}
	return NULL;
}

int proc_proc_readi(struct inode *ip, char *dst, int offset, int num)
{
	if(ip->type == T_DIR)
		return -1;
	int rd = 0;
	int pid = ip->inum >> 16;
	int low = ip->inum & 0xffff;
	struct proc *p = get_proc(pid);
	if(!p)
		return -1;
	if(low == 2) {
		rd += sprintf(dst, "%d %d %d %d %d %d\n", 
				p->pid, 
				p->parent ? p->parent->pid : 0,
				p->vend - USER_LINK,
				p->stat,
				p->ticks,
				p->priority
			);
	}
	else if(low == 3){
		rd += sprintf(dst, p->name);
	}
	return rd;
}

int proc_file_read(struct file *f, char *dst, int len);
int proc_file_write(struct file *f, char *src, int len);

struct file_operation proc_proc_file_op = {
	proc_file_read,
	proc_file_write,
	proc_proc_readdir,
};

struct inode_operation proc_proc_inode_op = {
	.f_op = &proc_proc_file_op,
	.readi = proc_proc_readi,
	.lookup = proc_proc_lookup,
};

