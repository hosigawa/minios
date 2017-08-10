#include "kernel.h"

struct block_buf bufs[BLOCK_BUF_NUM];
struct block_buf *head;
struct super_block sb;
struct inode inodes[INODE_NUM];

void binit()
{
	int i;
	bufs->next = bufs;
	bufs->prev = bufs;
	head = bufs;
	for(i = 1; i < BLOCK_BUF_NUM; i++){
		head->next->prev = bufs + i;
		bufs[i].next = head->next;
		head->next = bufs + i;
		bufs[i].prev = head;
	}
}

struct block_buf *bread(int dev, int sec)
{
	struct block_buf *buf = bget(dev, sec);
	if(!(buf->flags & B_VALID))
		ide_rw(buf);
	return buf;
}

void bwrite(int dev, int sec)
{
	struct block_buf *buf = bget(dev, sec);
	buf->flags |= B_DIRTY;
	ide_rw(buf);
}

struct block_buf *bget(int dev, int sec)
{
	struct block_buf *buf = NULL;
	for(buf = head; buf->next != head; buf = buf->next){
		if(buf->dev == dev && buf->sec == sec) {
			buf->ref++;
			return buf;
		}
	}

	for(buf = head; buf->prev != head; buf = buf->prev){
		if(buf->ref == 0 && !(buf->flags & B_DIRTY)) {
			buf = head->prev;
			buf->flags = 0;
			buf->dev = dev;
			buf->sec = sec;
			buf->ref = 1;
			return buf;
		}
	}
	panic("buffer block not enough\n");
	return NULL;
}

void brelse(struct block_buf *buf)
{
	if(--buf->ref <= 0) {
		buf->next->prev = buf->prev;
		buf->prev->next = buf->next;
		head->next->prev = buf;
		buf->next = head->next;
		buf->prev = head;
		head->next = buf;
	}
}

void readsb(int dev, struct super_block *sb) 
{
	struct block_buf *buf = bread(dev, 1);
	memmove(sb, buf->data, sizeof(struct super_block));
	brelse(buf);
}

void init_fs(int dev)
{
	binit();
	extern bool disk1;
	if(disk1) {
		readsb(dev, &sb);
		sys_info("load disk 1; size:%d, nblocks:%d, ninodes:%d, nlog:%d, logstart:%d, inodestart:%d, bmapstart:%d\n",
				sb.size, sb.nblocks, sb.ninodes, sb.nlog, sb.logstart, sb.inodestart, sb.bmapstart);
	}
}

struct inode *iget(int dev, int inum)
{
	int i = 0;
	for(; i < INODE_NUM; i++){
		if(inodes[i].dev == dev && inodes[i].inum == inum){
			inodes[i].ref++;
			return inodes + i;
		}

	}
}

void irelese(struct inode *ip)
{
}

void readi(struct inode *ip, char *dst)
{
}

void writei(struct inode *ip, char *dst)
{
}

int get_block_num(struct inode *ip, int n)
{
}

