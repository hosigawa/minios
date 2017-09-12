#include "kernel.h"

struct inode inodes[INODE_NUM];

struct super_block *get_sb(int dev);

struct inode *iget(int dev, int inum)
{
	int i = 0;
	struct inode *empty = NULL;
	for(; i < INODE_NUM; i++){
		if(inodes[i].ref > 0 && inodes[i].dev == dev && inodes[i].inum == inum){
			inodes[i].ref++;
			return inodes + i;
		}
		if(!empty && inodes[i].ref == 0){
			empty = inodes + i;
		}
	}
	if(!empty)
		panic("no empty inodes\n");
	empty->dev = dev;
	empty->inum = inum;
	empty->ref = 1;
	empty->flags = 0;
	empty->sb = get_sb(dev);
	if(empty->sb) {
		empty->i_op = empty->sb->i_op;
		empty->sb->s_op->read_inode(empty);
	}
	
	return empty;
}

void iput(struct inode *ip)
{
	if(--ip->ref < 0)
		panic("iput\n");
	if(ip->ref == 0 && (ip->flags & I_VALID) && ip->de.nlink == 0) {
		ip->de.type = 0;
		ip->flags = 0;
		ip->i_op->itrunc(ip);
		ip->sb->s_op->write_inode(ip);
		ip->sb = NULL;
	}
}

struct inode *idup(struct inode *ip)
{
	if(ip->ref < 1)
		panic("idup\n");
	ip->ref++;
	return ip;
}

