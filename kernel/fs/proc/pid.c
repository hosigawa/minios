#include "kernel.h"

struct proc_struct pid_struct[] = {
	{1, "."},
	{1, ".."},
	{2, "fd"},
	{3, "stat"},
	{4, "cmdline"},
	{5, "cwd"},
	{6, "exe"},
};

int proc_proc_readdir(struct file *f, struct dirent *de)
{
	struct inode *dp = f->de->ip;
	int pid = dp->inum >> 16;
	if(dp->type != T_DIR)
		return -1;
	struct proc_struct *ps = get_proc_struct(pid_struct, SIZEOF_STRUCT(pid_struct), f->off);
	if(!ps)
		return -1;
	de->inum = pid << 16 | ps->base;
	sprintf(de->name, ps->name);
	f->off++;
	return 0;
}

struct dentry *proc_proc_lookup(struct inode *dp, struct dentry *de, char *name, int *off)
{
	int pid = dp->inum >> 16;
	struct proc_struct *ps = match_proc_struct(pid_struct, SIZEOF_STRUCT(pid_struct), name);
	if(!ps)
		return NULL;
	return dget(de, name, dp->sb, pid << 16 | ps->base);
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
	if(low == 3) {
		rd += sprintf(dst, "%d %d %d %d %d %d\n", 
				p->pid, 
				p->parent ? p->parent->pid : 0,
				p->vend - USER_LINK,
				p->stat,
				p->ticks,
				p->priority
			);
	}
	else if(low == 4){
		rd += sprintf(dst, p->cmdline);
	}
	else if(low == 5) {
		d_path(p->cwd, dst);
	}
	else if(low == 6) {
		d_path(p->exe, dst);
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

int proc_fd_readdir(struct file *f, struct dirent *de)
{
	struct inode *dp = f->de->ip;
	int pid = dp->inum >> 16;
	if(f->off == 0) {
		sprintf(de->name, ".");
	}
	else if(f->off == 1) {
		sprintf(de->name, "..");
	}
	else {
		if(dp->type != T_DIR)
			return -1;
		struct proc *p = get_proc(pid);
		if(!p)
			return -1;
		if(!p->ofile[f->off - 2])
			return -1;
		sprintf(de->name, "%d", f->off - 2);
	}
	f->off++;
	return 0;
}

struct dentry *proc_fd_lookup(struct inode *dp, struct dentry *de, char *name, int *off)
{
	int pid = dp->inum >> 16;
	struct proc *p = get_proc(pid);
	if(!p)
		return NULL;
	int fd = atoi(name);
	if(fd < 0 || fd >= NOFILE)
		return NULL;
	if(!p->ofile[fd])
		return NULL;
	return dget(de, name, dp->sb, pid << 16 | (fd+1) << 8);
}

int proc_fd_readi(struct inode *ip, char *dst, int offset, int num)
{
	int pid = ip->inum >> 16;
	struct proc *p = get_proc(pid);
	if(!p)
		return -1;
	int fd = ((ip->inum & 0xff00) >> 8) - 1;
	if(!p->ofile[fd])
		return -1;
	d_path(p->ofile[fd]->de, dst);
	return 0;
}

struct file_operation proc_fd_file_op = {
	proc_file_read,
	proc_file_write,
	proc_fd_readdir,
};

struct inode_operation proc_fd_inode_op = {
	.f_op = &proc_fd_file_op,
	.readi = proc_fd_readi,
	.lookup = proc_fd_lookup,
};

