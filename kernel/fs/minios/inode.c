#include "kernel.h"

uint minios_bmap(struct super_block *sb, struct inode *ip, int n);
void minios_bfree(struct super_block *sb, uint n);
void minios_write_inode(struct super_block *sb, struct inode *ip);
void minios_read_inode(struct super_block *sb, struct inode *ip);
void minios_dirlink(struct inode *dp, char *name, int inum);
struct dentry *minios_lookup(struct inode *ip, struct dentry *de, char *name, int *off);
struct inode *minios_ialloc(struct super_block *sb, int type);

int minios_readi(struct inode *ip, char *dst, int offset, int num)
{
	struct block_buf *buf;
	int rd, intr;
	if(offset + num > ip->size)
		num = ip->size - offset;
	for(rd = 0, intr = 0; rd < num; rd += intr, offset += intr, dst += intr) {
		buf = bread(ip->dev, minios_bmap(ip->sb, ip, offset/BLOCK_SIZE));
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
	if(offset > ip->size)
		return -1;
	for(wt = 0, intr = 0; wt < num; wt += intr, offset += intr, src += intr) {
		buf = bread(ip->dev, minios_bmap(ip->sb, ip, offset/BLOCK_SIZE));
		intr = min(BLOCK_SIZE - offset % BLOCK_SIZE, num - wt);
		memmove(buf->data + offset % BLOCK_SIZE, src, intr);
		bwrite(buf);
		
		brelse(buf);
	}
	if(offset > ip->size) {
		ip->size = offset;
		minios_write_inode(ip->sb, ip);
	}
	return wt;
}

struct dentry *minios_create(struct inode *dp, struct dentry *de, char *name, int type, int major, int minor)
{
	struct dentry *new;
	struct inode *ip;
	int off;

	new = minios_lookup(dp, de, name, &off);
	if(new) {
		minios_read_inode(de->ip->sb, de->ip);
		if(type == T_FILE && de->ip->type == T_FILE)
			return new;
		dput(new);
		return NULL;
	}

	ip = minios_ialloc(dp->sb, type);
	if(!ip)
		panic("create file alloc inode error\n");

	ip->minios_i.major = major;
	ip->minios_i.minor = minor;
	ip->size = 0;
	ip->nlink = 1;
	ip->ctime = get_systime();
	ip->mtime = get_systime();
	ip->atime = get_systime();
	minios_write_inode(ip->sb, ip);

	if(type == T_DIR){
		dp->nlink++;
		minios_write_inode(dp->sb, dp);
		minios_dirlink(ip, ".", ip->inum);
		minios_dirlink(ip, "..", dp->inum);
	}

	minios_dirlink(dp, name, ip->inum);
	new = dalloc(de, name);
	dadd(new, ip);

	return new;
}

void minios_dirlink(struct inode *dp, char *name, int inum)
{
	if(dp->type != T_DIR) {
		panic("%s is not direct\n", name);
	}
	struct dirent de;
	int off = 0;
	for(; off < dp->size; off += sizeof(struct dirent)) {
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
	for(off = 2 * sizeof(de); off < dp->size; off += sizeof(de)) {
		if(minios_readi(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
			panic("dir_empty\n");
		if(de.inum != 0)
			return false;
	}
	return true;
}

int minios_unlink(struct inode *dp, struct dentry *de, char *name)
{
	struct dentry *sub;
	int off;

	sub = minios_lookup(dp, de, name, &off);
	if(!sub) {
		return -3;
	}

	if(sub->ip->nlink < 1) {
		dput(sub);
		return -4;
	}

	if(sub->ip->type == T_DIR && !minios_dir_empty(sub->ip)){
		dput(sub);
		return -5;
	}

	struct dirent dir;
	memset(&dir, 0, sizeof(dir));
	minios_writei(dp, (char *)&dir, off, sizeof(dir));
	if(de->ip->type == T_DIR) {
		dp->nlink--;
		minios_write_inode(dp->sb, dp);
	}

	sub->ip->nlink--;
	minios_write_inode(sub->ip->sb, sub->ip);
	dput(sub);

	return 0;
}

struct dentry *minios_lookup(struct inode *dp, struct dentry *de, char *name, int *off)
{
	if(dp->type != T_DIR) {
		panic("%s is not direct\n", name);
		return NULL;
	}
	struct dirent di;
	*off = 0;
	for(; *off < dp->size; *off += sizeof(struct dirent)) {
		minios_readi(dp, (char *)&di, *off, sizeof(struct dirent));
		if(di.inum == 0)
			continue;
		if(strcmp(di.name, name) == 0) {
			return dget(de, name, dp->sb, di.inum);
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
		if(ip->minios_i.addrs[i]) {
			minios_bfree(ip->sb, ip->minios_i.addrs[i]);
			ip->minios_i.addrs[i] = 0;
		}
	}
	if(ip->minios_i.addrs[NDIRECT]) {
		buf = bread(ip->dev, ip->minios_i.addrs[NDIRECT]);
		p = (uint *)buf->data;
		for(i = 0; i < BLOCK_SIZE / sizeof(uint); i++) {
			if(p[i])
				minios_bfree(ip->sb, p[i]);
		}
		brelse(buf);
		minios_bfree(ip->sb, ip->minios_i.addrs[NDIRECT]);
		ip->minios_i.addrs[NDIRECT] = 0;
	}

	ip->size = 0;
	minios_write_inode(ip->sb, ip);
}

extern struct file_operation minios_file_op;
struct inode_operation minios_inode_op = {
	&minios_file_op,
	minios_lookup,
	minios_itrunc,
	minios_dirlink,
	minios_create,
	minios_unlink,
	minios_readi,
	minios_writei,
};

