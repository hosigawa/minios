#include "kernel.h"

struct block_buf bufs[BLOCK_BUF_NUM];
struct block_buf *head;
struct super_block sb;
struct inode inodes[INODE_NUM];

uint balloc(int dev);
void bfree(int dev, uint n);
void fill_inode(struct inode *ip);
struct inode *dirlookup(struct inode *ip, const char *name);

void bzero(int dev, int num)
{
	struct block_buf *buf = bread(dev, num);
	memset(buf->data, 0, BLOCK_SIZE);
	bwrite(buf);
	brelse(buf);
}

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

void bwrite(struct block_buf *buf)
{
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
		namei("init");
	}
}

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
	
	return empty;
}

void irelese(struct inode *ip)
{
	if(--ip->ref <= 0) {

	}
}

int readi(struct inode *ip, char *dst, int offset, int num)
{
	struct block_buf *buf;
	int rd, intr;
	if(offset + num > ip->di.size)
		num = ip->di.size - offset;
	for(rd = 0, intr = 0; rd < num; rd += intr, offset += intr, dst += intr) {
		buf = bread(ip->dev, bmap(ip, offset/BLOCK_SIZE));
		intr = min(BLOCK_SIZE - offset % BLOCK_SIZE, num - rd);
		memmove(dst, buf->data + offset % BLOCK_SIZE, intr);
		brelse(buf);
	}
	return rd;
}

int writei(struct inode *ip, char *dst, int offset, int num)
{
	return 0;
}

uint bmap(struct inode *ip, int n)
{
	if(n < NDIRECT){
		if(ip->di.addrs[n] == 0){
			ip->di.addrs[n] = balloc(ip->dev);
		}
		return ip->di.addrs[n];
	}
	n -= NDIRECT;
	if(ip->di.addrs[NDIRECT] == 0)
		ip->di.addrs[NDIRECT] = balloc(ip->dev);
	struct block_buf *buf = bread(ip->dev, ip->di.addrs[NDIRECT]);
	uint seq = *((uint*)buf->data + n);
	if(seq == 0) {
		*((uint *)buf->data + n) = seq = balloc(ip->dev);
		bwrite(buf);
	}
	brelse(buf);
	return seq;
}

void fill_inode(struct inode *ip)
{
	if(!(ip->flags & I_VALID)) {
		struct block_buf *buf = bread(ip->dev, IBLOCK(ip->inum));
		memmove(&ip->di, (struct dinode *)buf->data + ip->inum % IPER, sizeof(struct dinode));
		ip->flags |= I_VALID;
		brelse(buf);
	}
}

uint balloc(int dev)
{
	int i, bi, m;
	for(i = 0; i < sb.size; i += BPER) {
		struct block_buf *buf = bread(dev, BBLOCK(i));
		for(bi = 0; bi < BPER && bi + i < sb.size; bi++) {
			m = 1 << (bi % 8);
			if((buf->data[bi/8] & m) == 0){
				buf->data[bi/8] |= m;
				bwrite(buf);
				bzero(dev, i + bi);
				brelse(buf);
				return bi;;
			}
		}
		brelse(buf);
	}
	panic("balloc out of range\n");
	return 0;
}

void bfree(int dev, uint n)
{
	struct block_buf *buf = bread(dev, BBLOCK(n));
	int b = n % BPER;
	int m = 1 << (b % 8);
	buf->data[b/8] &= ~m;
	bwrite(buf);
	brelse(buf);
}

struct inode *namei(const char *path)
{
	struct inode *dp = iget(1, 1);
	fill_inode(dp);
	struct inode *ip = dirlookup(dp, path);
	if(ip) {
		fill_inode(ip);
		char data[ip->di.size];
		readi(ip, data, 0, ip->di.size);
		printf("data:\n%s\n", data);
	}
	else {
		printf("can't find %s\n", path);
	}
	irelese(ip);
	return ip;
}

struct inode *dirlookup(struct inode *ip, const char *name)
{
	if(ip->di.type != T_DIR) {
		panic("%s is not direct\n", name);
		return NULL;
	}
	struct dirent de;
	int off = 0;
	for(; off < ip->di.size; off += sizeof(struct dirent)) {
		readi(ip, (char *)&de, off, sizeof(struct dirent));
		if(de.inum == 0)
			continue;
		if(strncmp(de.name, name, DIR_NM_SZ) == 0) {
			return iget(ip->dev, de.inum);
		}
	}
	return NULL;
}

