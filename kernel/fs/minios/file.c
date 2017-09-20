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
	struct inode *ip = f->de->ip;
	if(ip->type == T_DEV && devrw[ip->minios_i.major].read) {
		ret = devrw[ip->minios_i.major].read(ip, dst, f->off, len);
	}
	else {
		if(ip->type == T_DIR)
			return -1;
		ret = minios_readi(ip, dst, f->off, len);
		ip->atime = get_systime();
		minios_write_inode(ip->sb, ip);
	}
	f->off += ret;

	return ret;
}

int minios_file_write(struct file *f, char *src, int len)
{
	int ret;
	struct inode *ip = f->de->ip;
	if(ip->type == T_DEV && devrw[ip->minios_i.major].write) {
		ret = devrw[ip->minios_i.major].write(ip, src, f->off, len);
	}
	else {
		if(ip->type == T_DIR)
			return -1;
		ret = minios_writei(ip, src, f->off, len);
		ip->mtime = get_systime();
		minios_write_inode(ip->sb, ip);
	}
	f->off += ret;

	return ret;
}

int minios_file_readdir(struct file *f, struct dirent *de)
{
	int ret = 0;
	while(1) {
		ret = minios_readi(f->de->ip, (char *)de, f->off, sizeof(struct dirent));
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

