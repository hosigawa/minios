#include "kernel.h"

extern struct CPU cpu;

struct file file_table[NFILE];

void init_file() 
{
	int i;
	for(i = 0; i < NFILE; i++) {
		file_table[i].ref = 0;
	}
}

struct file *get_file(int fd)
{
	struct file *f;
	if(fd > NOFILE || fd < 0 || (f = cpu.cur_proc->ofile[fd]) == NULL) {
		return NULL;
	}
	return f;
}

struct file *file_dup(struct file *f)
{
	if(f->ref <= 0)
		panic("file_dup\n");
	f->ref++;
	return f;
}

struct file *file_alloc()
{
	int i;
	for(i = 0; i < NFILE; i++) {
		if(file_table[i].ref == 0) {
			file_table[i].ref = 1;
			return file_table + i;
		}
	}
	panic("file_table max\n");
	return NULL;
}

int fd_alloc(struct file *f)
{
	int i;
	for(i = 0; i < NOFILE; i++){
		if(cpu.cur_proc->ofile[i] == NULL) {
			cpu.cur_proc->ofile[i] = f;
			return i;
		}
	}
	return -1;
}

int file_mknod(char *path, int major, int minor) 
{
	struct dentry *de = file_create(path, T_DEV, major, minor);
	if(!de)
		return -1;
	dput(de);
	return 0;
}

int file_mkdir(char *path, int major, int minor)
{
	struct dentry *de = file_create(path, T_DIR, major, minor);
	if(!de)
		return -1;
	dput(de);
	return 0;
}

int file_close(struct file *f)
{
	if(--f->ref == 0) {
		if(f->type == FD_INODE)
			dput(f->de);
		f->type = FD_NONE;
	}
	return 0;
}

int file_open(char *path, int mode)
{
	int fd;
	struct file *f;
	struct dentry *dp;
	if(mode & O_CREATE) {
		dp = file_create(path, T_FILE, 0, 0);
	}
	else {
		dp = namei(path);
		if(!dp) {
			return -1;
		}
		if(dp->ip->type == T_DIR && mode > 0) {
			dput(dp);
			return -2;
		}
	}

	struct super_block *sb = dp->ip->sb;

	f = file_alloc();
	f->f_op = dp->ip->i_op->f_op;
	f->type = FD_INODE;
	f->de = dp;
	f->off = 0;

	fd = fd_alloc(f);
	if(fd < 0){
		file_close(f);
		return -3;
	}
	f->de->ip->atime = get_systime();
	sb->s_op->write_inode(sb, f->de->ip);
	return fd;
}

struct dentry *file_create(char *path, int type, int major, int minor)
{
	struct dentry *dp, *ip;
	char name[DIR_NM_SZ];
	dp = namep(path, name);
	if(!dp) {
		return NULL;
	}
	if(dp->ip->type != T_DIR) {
		dput(dp);
		return NULL;
	}

	ip = dp->ip->i_op->create(dp->ip, dp, name, type, major, minor);
	dput(dp);

	return ip;
}

int file_unlink(char *path)
{
	struct dentry *dp;
	char name[DIR_NM_SZ];
	dp = namep(path, name);
	if(!dp)
		return -1;
	
	if(strcmp(name, "..") == 0 || strcmp(name, ".") == 0) {
		dput(dp);
		return -2;
	}

	int ret = dp->ip->i_op->unlink(dp->ip, dp, name);
	dput(dp);
	return ret;
}

