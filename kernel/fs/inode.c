#include "kernel.h"

struct inode inodes[INODE_NUM];

struct super_block *get_sb(int dev);

struct inode *iget(struct super_block *sb, int inum)
{
	int i = 0;
	struct inode *empty = NULL;
	for(; i < INODE_NUM; i++){
		if(inodes[i].ref > 0 && inodes[i].dev == sb->dev && inodes[i].inum == inum){
			inodes[i].ref++;
			return inodes + i;
		}
		if(!empty && inodes[i].ref == 0){
			empty = inodes + i;
		}
	}
	if(!empty)
		panic("no empty inodes\n");
	empty->ref = 1;
	empty->flags = 0;
	empty->dev = sb->dev;
	empty->inum = inum;
	empty->sb = sb;
	sb->s_op->read_inode(sb, empty);

	return empty;
}

void iput(struct inode *ip)
{
	if(!ip || --ip->ref < 0)
		panic("iput error\n");
	if(ip->ref == 0) {
		if((ip->flags & I_VALID) && ip->nlink == 0) {
			ip->type = 0;
			ip->flags = 0;
			if(ip->i_op->itrunc)
				ip->i_op->itrunc(ip);
			ip->sb->s_op->write_inode(ip->sb, ip);
		}
		ip->sb = NULL;
	}
}

struct inode *idup(struct inode *ip)
{
	if(!ip || ip->ref < 1)
		panic("idup error\n");
	ip->ref++;
	return ip;
}

