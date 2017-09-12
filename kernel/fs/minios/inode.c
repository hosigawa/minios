#include "kernel.h"

uint minios_bmap(struct inode *ip, int n);
void minios_bfree(int dev, uint n);
void minios_write_inode(struct inode *ip);
void minios_read_inode(struct inode *ip);
void minios_dir_link(struct inode *dp, char *name, int inum);
struct inode *minios_dir_lookup(struct inode *ip, char *name, int *off);
struct inode *minios_ialloc(int dev, int type);

int minios_readi(struct inode *ip, char *dst, int offset, int num)
{
	struct block_buf *buf;
	int rd, intr;
	if(offset + num > ip->de.size)
		num = ip->de.size - offset;
	for(rd = 0, intr = 0; rd < num; rd += intr, offset += intr, dst += intr) {
		buf = bread(ip->dev, minios_bmap(ip, offset/BLOCK_SIZE));
		intr = min(BLOCK_SIZE - offset % BLOCK_SIZE, num - rd);
		memmove(dst, buf->data + offset % BLOCK_SIZE, intr);
		brelse(buf);
	}
	return rd;
}

int minios_writei(struct inode *ip, char *src, int offset, int num)
{
	struct block_buf *buf;
	int wt, intr;
	if(offset > ip->de.size)
		return -1;
	for(wt = 0, intr = 0; wt < num; wt += intr, offset += intr, src += intr) {
		buf = bread(ip->dev, minios_bmap(ip, offset/BLOCK_SIZE));
		intr = min(BLOCK_SIZE - offset % BLOCK_SIZE, num - wt);
		memmove(buf->data + offset % BLOCK_SIZE, src, intr);
		bwrite(buf);
		
		brelse(buf);
	}
	if(offset > ip->de.size) {
		ip->de.size = offset;
		minios_write_inode(ip);
	}
	return wt;
}

struct inode *minios_create(struct inode *dp, char *name, int type, int major, int minor)
{
	struct inode *ip;
	int off;

	ip = minios_dir_lookup(dp, name, &off);
	if(ip) {
		minios_read_inode(ip);
		if(type == T_FILE && ip->de.type == T_FILE)
			return ip;
		iput(ip);
		return NULL;
	}

	ip = minios_ialloc(dp->dev, type);
	if(!ip)
		panic("create file alloc inode error\n");

	ip->de.major = major;
	ip->de.minor = minor;
	ip->de.size = 0;
	ip->de.nlink = 1;
	ip->de.ctime = get_systime();
	ip->de.mtime = get_systime();
	ip->de.atime = get_systime();
	minios_write_inode(ip);

	if(type == T_DIR){
		dp->de.nlink++;
		minios_write_inode(dp);
		minios_dir_link(ip, ".", ip->inum);
		minios_dir_link(ip, "..", dp->inum);
	}

	minios_dir_link(dp, name, ip->inum);

	return ip;
}

void minios_dir_link(struct inode *dp, char *name, int inum)
{
	if(dp->de.type != T_DIR) {
		panic("%s is not direct\n", name);
	}
	struct dirent de;
	int off = 0;
	for(; off < dp->de.size; off += sizeof(struct dirent)) {
		minios_readi(dp, (char *)&de, off, sizeof(struct dirent));
		if(de.inum == 0)
			break;
	}
	memmove(de.name, name, DIR_NM_SZ);
	de.inum = inum;

	int ret = minios_writei(dp, (char *)&de, off, sizeof(struct dirent));
	if(ret != sizeof(struct dirent))
		panic("dir_link\n");
}

bool minios_dir_empty(struct inode *dp)
{
	int off;
	struct dirent de;
	for(off = 2 * sizeof(de); off < dp->de.size; off += sizeof(de)) {
		if(minios_readi(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
			panic("dir_empty\n");
		if(de.inum != 0)
			return false;
	}
	return true;
}

int minios_unlink(struct inode *dp, char *name)
{
	struct inode *ip;
	int off;

	ip = minios_dir_lookup(dp, name, &off);
	if(!ip) {
		return -3;
	}

	if(ip->de.nlink < 1) {
		iput(ip);
		return -4;
	}

	if(ip->de.type == T_DIR && !minios_dir_empty(ip)){
		iput(ip);
		return -5;
	}

	struct dirent de;
	memset(&de, 0, sizeof(de));
	minios_writei(dp, (char *)&de, off, sizeof(de));
	if(ip->de.type == T_DIR) {
		dp->de.nlink--;
		minios_write_inode(dp);
	}

	ip->de.nlink--;
	minios_write_inode(ip);
	iput(ip);

	return 0;
}

struct inode *minios_dir_lookup(struct inode *ip, char *name, int *off)
{
	if(ip->de.type != T_DIR) {
		panic("%s is not direct\n", name);
		return NULL;
	}
	struct dirent de;
	*off = 0;
	for(; *off < ip->de.size; *off += sizeof(struct dirent)) {
		minios_readi(ip, (char *)&de, *off, sizeof(struct dirent));
		if(de.inum == 0)
			continue;
		if(strcmp(de.name, name) == 0) {
			return iget(ip->dev, de.inum);
		}
	}
	return NULL;
}

void minios_itrunc(struct inode *ip)
{
	struct block_buf *buf;
	uint *p;
	int i;
	for(i = 0; i < NDIRECT; i++) {
		if(ip->de.addrs[i]) {
			minios_bfree(ip->dev, ip->de.addrs[i]);
			ip->de.addrs[i] = 0;
		}
	}
	if(ip->de.addrs[NDIRECT]) {
		buf = bread(ip->dev, ip->de.addrs[NDIRECT]);
		p = (uint *)buf->data;
		for(i = 0; i < BLOCK_SIZE / sizeof(uint); i++) {
			if(p[i])
				minios_bfree(ip->dev, p[i]);
		}
		brelse(buf);
		minios_bfree(ip->dev, ip->de.addrs[NDIRECT]);
		ip->de.addrs[NDIRECT] = 0;
	}

	ip->de.size = 0;
	minios_write_inode(ip);
}

struct inode_operation minios_inode_op = {
	minios_dir_lookup,
	minios_itrunc,
	minios_dir_link,
	minios_create,
	minios_unlink,
	minios_readi,
	minios_writei,
};

