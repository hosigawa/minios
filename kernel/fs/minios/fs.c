#include "kernel.h"
#include "minios.h"

uint minios_balloc(struct super_block *sb);
void bzero(int dev, int num);

uint minios_bmap(struct super_block *sb, struct inode *ip, int n)
{
	if(n < NDIRECT){
		if(ip->minios_i.addrs[n] == 0){
			ip->minios_i.addrs[n] = minios_balloc(sb);
		}
		return ip->minios_i.addrs[n];
	}
	n -= NDIRECT;
	if(ip->minios_i.addrs[NDIRECT] == 0)
		ip->minios_i.addrs[NDIRECT] = minios_balloc(sb);
	struct block_buf *buf = bread(ip->dev, ip->minios_i.addrs[NDIRECT]);
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
	for(i = 0; i < sb->minios_s.size; i += BPER) {
		struct block_buf *buf = bread(sb->dev, BBLOCK(i,sb));
		for(bi = 0; bi < BPER && bi + i < sb->minios_s.size; bi++) {
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
	struct minios_inode *dp;
	int dev = sb->dev;

	for(off = 0; off < sb->minios_s.ninodes; off += IPER) {
		buf = bread(dev, IBLOCK(off,sb));
		for(i = 0; i < IPER; i++) {
			if(off == 0 && i == 0)
				continue;
			dp = (struct minios_inode *)buf->data + i;
			if(dp->type == 0) {
				memset(dp, 0, sizeof(*dp));
				dp->type = type;
				bwrite(buf);
				brelse(buf);
				return iget(sb, off + i);
			}
		}
		brelse(buf);
	}
	panic("dinode max\n");
}

void minios_read_inode(struct super_block *sb, struct inode *ip)
{
	struct minios_inode *mi;
	if(!(ip->flags & I_VALID)) {
		struct block_buf *buf = bread(sb->dev, IBLOCK(ip->inum,sb));
		mi = (struct minios_inode *)buf->data + ip->inum % IPER;
		ip->flags |= I_VALID;
		ip->i_op = sb->i_op;
		ip->type = mi->type;
		ip->ctime = mi->ctime;
		ip->mtime = mi->mtime;
		ip->atime = mi->atime;
		ip->size = mi->size;
		ip->nlink = mi->nlink;
		int i;
		for(i = 0; i <= NDIRECT; i++)
			ip->minios_i.addrs[i] = mi->addrs[i];
		if(ip->type == T_DEV){
			ip->dev = mi->addrs[0];
			ip->i_op = get_devop(MAJOR(ip->dev));
		}
		brelse(buf);
	}
}

void minios_write_inode(struct super_block *sb, struct inode *ip)
{
	struct minios_inode *mi;
	struct block_buf *buf;
	
	buf = bread(sb->dev, IBLOCK(ip->inum,sb));
	mi = (struct minios_inode *)buf->data + ip->inum % IPER;
	mi->type = ip->type;
	mi->ctime = ip->ctime;
	mi->mtime = ip->mtime;
	mi->atime = ip->atime;
	mi->size = ip->size;
	mi->nlink = ip->nlink;
	int i;
	for(i = 0; i <= NDIRECT; i++)
		mi->addrs[i] = ip->minios_i.addrs[i];
	if(ip->type == T_DEV) {
		mi->addrs[0] = ip->dev;
	}
	bwrite(buf);
	brelse(buf);
}

void minios_read_sb(struct super_block *sb); 
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
}


extern struct file_operation minios_file_op;
extern struct inode_operation minios_inode_op;
void minios_read_sb(struct super_block *sb) 
{
	struct block_buf *buf = bread(sb->dev, ROOT_INO);
	memmove(&sb->minios_s, buf->data, sizeof(struct minios_super_block));
	brelse(buf);
	sb->s_op = &minios_sb_op;
	sb->i_op = &minios_inode_op;
}

