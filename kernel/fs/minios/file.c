#include "kernel.h"

void minios_read_inode(struct super_block *sb, struct inode *ip);
void minios_write_inode(struct super_block *sb, struct inode *ip);
int minios_readi(struct inode *ip, char *dst, int offset, int num);
int minios_writei(struct inode *ip, char *src, int offset, int num);

typedef int (*rw_func)(struct inode *, char *, int, int);

struct devrw {
	rw_func read;
	rw_func write; 
};

struct devrw devrw[MAX_DEV];

int register_devrw(int dev, rw_func read, rw_func write)
{
	devrw[dev].read = read;
	devrw[dev].write = write;
	return 0;
}

void init_dev()
{
	register_devrw(CONSOLE, console_read, console_write);
}

int minios_file_read(struct file *f, char *dst, int len)
{
	int ret;
	if(f->ip->type == T_DEV && devrw[f->ip->minios_i.major].read) {
		ret = devrw[f->ip->minios_i.major].read(f->ip, dst, f->off, len);
	}
	else {
		if(f->ip->type == T_DIR)
			return -1;
		ret = minios_readi(f->ip, dst, f->off, len);
		f->ip->atime = get_systime();
		minios_write_inode(f->ip->sb, f->ip);
	}
	f->off += ret;

	return ret;
}

int minios_file_write(struct file *f, char *src, int len)
{
	int ret;
	if(f->ip->type == T_DEV && devrw[f->ip->minios_i.major].write) {
		ret = devrw[f->ip->minios_i.major].write(f->ip, src, f->off, len);
	}
	else {
		if(f->ip->type == T_DIR)
			return -1;
		ret = minios_writei(f->ip, src, f->off, len);
		f->ip->mtime = get_systime();
		minios_write_inode(f->ip->sb, f->ip);
	}
	f->off += ret;

	return ret;
}

int minios_file_readdir(struct file *f, struct dirent *de)
{
	int ret = 0;
	while(1) {
		ret = minios_readi(f->ip, (char *)de, f->off, sizeof(struct dirent));
		if(ret == sizeof(struct dirent)) {
			f->off += ret;
			if(de->inum == 0)
				continue;
			return 0;
		}
		else
			return -1;
	}
	return 0;
}

struct file_operation minios_file_op = {
	minios_file_read,
	minios_file_write,
	minios_file_readdir,
};

