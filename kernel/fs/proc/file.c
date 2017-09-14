#include "kernel.h"

int proc_readi(struct inode *ip, char *dst, int offset, int num);
int proc_writei(struct inode *ip, char *src, int offset, int num);

int proc_file_read(struct file *f, char *dst, int len)
{
	int ret = f->ip->i_op->readi(f->ip, dst, f->off, len);
	f->off += ret;

	return ret;
}

int proc_file_write(struct file *f, char *src, int len)
{
	int ret = f->ip->i_op->writei(f->ip, src, f->off, len);
	f->off += ret;

	return ret;
}

struct file_operation proc_file_op = {
	proc_file_read,
	proc_file_write,
};

