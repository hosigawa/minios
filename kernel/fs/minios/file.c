#include "kernel.h"
#include "minios.h"

int minios_file_read(struct file *f, char *dst, int len)
{
	int ret;
	struct inode *ip = f->de->ip;
	ret = minios_readi(ip, dst, f->off, len);
	ip->atime = get_systime();
	minios_write_inode(ip->sb, ip);

	f->off += ret;

	return ret;
}

int minios_file_write(struct file *f, char *src, int len)
{
	int ret;
	struct inode *ip = f->de->ip;
	ret = minios_writei(ip, src, f->off, len);
	ip->mtime = get_systime();
	minios_write_inode(ip->sb, ip);

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

