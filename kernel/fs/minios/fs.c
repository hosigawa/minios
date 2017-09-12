#include "kernel.h"

uint minios_balloc(struct super_block *sb);
void bzero(int dev, int num);
void init_dev();

uint minios_bmap(struct super_block *sb, struct inode *ip, int n)
{
	if(n < NDIRECT){
		if(ip->de.addrs[n] == 0){
			ip->de.addrs[n] = minios_balloc(sb);
		}
		return ip->de.addrs[n];
	}
	n -= NDIRECT;
	if(ip->de.addrs[NDIRECT] == 0)
		ip->de.addrs[NDIRECT] = minios_balloc(sb);
	struct block_buf *buf = bread(ip->dev, ip->de.addrs[NDIRECT]);
	uint seq = *((uint*)buf->data + n);
	if(seq == 0) {
		*((uint *)buf->data + n) = seq = minios_balloc(sb);
		bwrite(buf);
	}
	brelse(buf);
	return seq;
}

uint minios_balloc(struct super_block *sb)
{
	int i, bi, m;
	for(i = 0; i < sb->size; i += BPER) {
		struct block_buf *buf = bread(sb->dev, BBLOCK(i,sb));
		for(bi = 0; bi < BPER && bi + i < sb->size; bi++) {
			m = 1 << (bi % 8);
			if((buf->data[bi/8] & m) == 0){
				buf->data[bi/8] |= m;
				bwrite(buf);
				bzero(sb->dev, i + bi);
				brelse(buf);
				return bi;;
			}
		}
		brelse(buf);
	}
	panic("balloc out of range\n");
	return 0;
}

void minios_bfree(struct super_block *sb, uint n)
{
	struct block_buf *buf = bread(sb->dev, BBLOCK(n,sb));
	int b = n % BPER;
	int m = 1 << (b % 8);
	if((buf->data[b/8] & m) == 0)
		panic("bfree free block\n");
	buf->data[b/8] &= ~m;
	bwrite(buf);
	brelse(buf);
}

struct inode *minios_ialloc(struct super_block *sb, int type)
{
	int off, i;
	struct block_buf *buf;
	struct dinode *dp;
	int dev = sb->dev;

	for(off = 0; off < sb->ninodes; off += IPER) {
		buf = bread(dev, IBLOCK(off,sb));
		for(i = 0; i < IPER; i++) {
			if(off == 0 && i == 0)
				continue;
			dp = (struct dinode *)buf->data + i;
			if(dp->type == 0) {
				memset(dp, 0, sizeof(*dp));
				dp->type = type;
				bwrite(buf);
				brelse(buf);
				return iget(dev, off + i);
			}
		}
		brelse(buf);
	}
	panic("dinode max\n");
}

void minios_read_inode(struct super_block *sb, struct inode *ip)
{
	if(!(ip->flags & I_VALID)) {
		struct block_buf *buf = bread(ip->dev, IBLOCK(ip->inum,sb));
		memmove(&ip->de, (struct dinode *)buf->data + ip->inum % IPER, sizeof(struct dinode));
		ip->flags |= I_VALID;
		brelse(buf);
	}
}

void minios_write_inode(struct super_block *sb, struct inode *ip)
{
	struct dinode *dp;
	struct block_buf *buf;
	buf = bread(ip->dev, IBLOCK(ip->inum,sb));
	dp = (struct dinode *)buf->data + ip->inum % IPER;
	memmove(dp, &ip->de, sizeof(*dp));
	bwrite(buf);
	brelse(buf);
}

void minios_read_sb(struct super_block *sb, int dev); 
struct super_block_operation minios_sb_op = {
	minios_read_sb,
	minios_ialloc,
	minios_read_inode,
	minios_write_inode,
};

void register_file_system_minios(struct file_system *fs)
{
	fs->reg = 1;
	strcpy(fs->name, "minios");
	fs->s_op = &minios_sb_op;
	init_dev();
}


extern struct file_operation minios_file_op;
extern struct inode_operation minios_inode_op;
void minios_read_sb(struct super_block *sb, int dev) 
{
	struct block_buf *buf = bread(dev, 1);
	memmove(sb, buf->data, 28);
	brelse(buf);
	sb->dev = dev;
	sb->s_op = &minios_sb_op;
	sb->i_op = &minios_inode_op;
	sb->f_op = &minios_file_op;
}

