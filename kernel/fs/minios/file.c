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
	register_devrw(PROCINFO, procinfo_read, procinfo_write);
	register_devrw(SYSINFO, sysinfo_read, sysinfo_write);
}

int minios_file_read(struct file *f, char *dst, int len)
{
	int ret;
	if(f->ip->de.type == T_DEV && devrw[f->ip->de.major].read) {
		ret = devrw[f->ip->de.major].read(f->ip, dst, f->off, len);
	}
	else {
		ret = minios_readi(f->ip, dst, f->off, len);
		f->ip->de.atime = get_systime();
		minios_write_inode(f->ip->sb, f->ip);
	}
	f->off += ret;

	return ret;
}

int minios_file_write(struct file *f, char *src, int len)
{
	int ret;
	if(f->ip->de.type == T_DEV && devrw[f->ip->de.major].write) {
		ret = devrw[f->ip->de.major].write(f->ip, src, f->off, len);
	}
	else {
		ret = minios_writei(f->ip, src, f->off, len);
		f->ip->de.mtime = get_systime();
		minios_write_inode(f->ip->sb, f->ip);
	}
	f->off += ret;

	return ret;
}

struct file_operation minios_file_op = {
	minios_file_read,
	minios_file_write,
};

