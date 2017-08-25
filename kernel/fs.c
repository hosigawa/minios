#include "kernel.h"

extern struct CPU cpu;

struct block_buf bufs[BLOCK_BUF_NUM];
struct block_buf *head;
struct super_block sb;
struct inode inodes[INODE_NUM];

uint balloc(int dev);
void bfree(int dev, uint n);
char *path_decode(char *path, char *name);

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
	if(--ip->ref < 0)
		panic("irelese\n");
	if(ip->ref == 0 && (ip->flags & I_VALID) && ip->de.nlink == 0) {
		ip->de.type = 0;
		ip->flags = 0;
		itrunc(ip);
		write_inode(ip);
	}
}

void itrunc(struct inode *ip)
{
	struct block_buf *buf;
	uint *p;
	int i;
	for(i = 0; i < NDIRECT; i++) {
		if(ip->de.addrs[i]) {
			bfree(ip->dev, ip->de.addrs[i]);
			ip->de.addrs[i] = 0;
		}
	}
	if(ip->de.addrs[NDIRECT]) {
		buf = bread(ip->dev, ip->de.addrs[NDIRECT]);
		p = (uint *)buf->data;
		for(i = 0; i < BLOCK_SIZE / sizeof(uint); i++) {
			if(p[i])
				bfree(ip->dev, p[i]);
		}
		brelse(buf);
		bfree(ip->dev, ip->de.addrs[NDIRECT]);
		ip->de.addrs[NDIRECT] = 0;
	}

	ip->de.size = 0;
	write_inode(ip);
}

int readi(struct inode *ip, char *dst, int offset, int num)
{
	struct block_buf *buf;
	int rd, intr;
	if(offset + num > ip->de.size)
		num = ip->de.size - offset;
	for(rd = 0, intr = 0; rd < num; rd += intr, offset += intr, dst += intr) {
		buf = bread(ip->dev, bmap(ip, offset/BLOCK_SIZE));
		intr = min(BLOCK_SIZE - offset % BLOCK_SIZE, num - rd);
		memmove(dst, buf->data + offset % BLOCK_SIZE, intr);
		brelse(buf);
	}
	return rd;
}

int writei(struct inode *ip, char *src, int offset, int num)
{
	struct block_buf *buf;
	int wt, intr;
	if(offset > ip->de.size)
		return -1;
	for(wt = 0, intr = 0; wt < num; wt += intr, offset += intr, src += intr) {
		buf = bread(ip->dev, bmap(ip, offset/BLOCK_SIZE));
		intr = min(BLOCK_SIZE - offset % BLOCK_SIZE, num - wt);
		memmove(buf->data + offset % BLOCK_SIZE, src, intr);
		bwrite(buf);
		
		brelse(buf);
	}
	if(offset > ip->de.size) {
		ip->de.size = offset;
		write_inode(ip);
	}
	return wt;
}

uint bmap(struct inode *ip, int n)
{
	if(n < NDIRECT){
		if(ip->de.addrs[n] == 0){
			ip->de.addrs[n] = balloc(ip->dev);
		}
		return ip->de.addrs[n];
	}
	n -= NDIRECT;
	if(ip->de.addrs[NDIRECT] == 0)
		ip->de.addrs[NDIRECT] = balloc(ip->dev);
	struct block_buf *buf = bread(ip->dev, ip->de.addrs[NDIRECT]);
	uint seq = *((uint*)buf->data + n);
	if(seq == 0) {
		*((uint *)buf->data + n) = seq = balloc(ip->dev);
		bwrite(buf);
	}
	brelse(buf);
	return seq;
}

void read_inode(struct inode *ip)
{
	if(!(ip->flags & I_VALID)) {
		struct block_buf *buf = bread(ip->dev, IBLOCK(ip->inum));
		memmove(&ip->de, (struct dinode *)buf->data + ip->inum % IPER, sizeof(struct dinode));
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
	if((buf->data[b/8] & m) == 0)
		panic("bfree free block\n");
	buf->data[b/8] &= ~m;
	bwrite(buf);
	brelse(buf);
}

struct inode *namex(char *path, char *name, bool bparent)
{
	struct inode *ip, *next;
	int off;
	if(*path == '/')
		ip = iget(1, 1);
	else
		ip = idup(cpu.cur_proc->cwd);

	while((path = path_decode(path, name)) != 0) {
		read_inode(ip);
		if(ip->de.type != T_DIR){
			err_info("ip isn't dir\n");
			irelese(ip);
			return NULL;
		}
		if(bparent && *path == 0) {
			return ip;
		}
		next = dir_lookup(ip, name, &off);
		if(!next) {
			irelese(ip);
			return NULL;
		}
		irelese(ip);
		ip = next;
	}
	if(bparent) {
		irelese(ip);
		return NULL;
	}
	return ip;
}

struct inode *namei(char *path)
{
	char name[DIR_NM_SZ];
	return namex(path, name, false);
}

struct inode *namep(char *path, char *name)
{
	return namex(path, name, true);
}

struct inode *dir_lookup(struct inode *ip, char *name, int *off)
{
	if(ip->de.type != T_DIR) {
		panic("%s is not direct\n", name);
		return NULL;
	}
	struct dirent de;
	*off = 0;
	for(; *off < ip->de.size; *off += sizeof(struct dirent)) {
		readi(ip, (char *)&de, *off, sizeof(struct dirent));
		if(de.inum == 0)
			continue;
		if(strcmp(de.name, name) == 0) {
			return iget(ip->dev, de.inum);
		}
	}
	return NULL;
}

char *path_decode(char *path, char *name)
{
	char *sub_path;
	if(*path == '/')
		path++;
	if(!*path)
		return NULL;
	sub_path = path;
	while(*path != '/' && *path)
		path++;
	int len = path - sub_path;
	if(len >= DIR_NM_SZ) {
		memmove(name, sub_path, DIR_NM_SZ);
	}
	else {
		memmove(name, sub_path, len);
		name[len] = 0;
	}
	if(*path == '/')
		path++;
	return path;
}

struct inode *ialloc(int dev, int type)
{
	int i;
	struct block_buf *buf;
	struct dinode *dp;

	for(i = 1; i < sb.ninodes; i++) {
		buf = bread(dev, IBLOCK(i));
		dp = (struct dinode *)buf->data + i % IPER;
		if(dp->type == 0) {
			memset(dp, 0, sizeof(*dp));
			dp->type = type;
			bwrite(buf);
			brelse(buf);
			return iget(dev, i);
		}
		brelse(buf);
	}
	panic("dinode max\n");
}

void write_inode(struct inode *ip)
{
	struct dinode *dp;
	struct block_buf *buf;
	buf = bread(ip->dev, IBLOCK(ip->inum));
	dp = (struct dinode *)buf->data + ip->inum % IPER;
	memmove(dp, &ip->de, sizeof(*dp));
	bwrite(buf);
	brelse(buf);
}

void dir_link(struct inode *dp, char *name, int inum)
{
	if(dp->de.type != T_DIR) {
		panic("%s is not direct\n", name);
	}
	struct dirent de;
	int off = 0;
	for(; off < dp->de.size; off += sizeof(struct dirent)) {
		readi(dp, (char *)&de, off, sizeof(struct dirent));
		if(de.inum == 0)
			break;
	}
	memmove(de.name, name, DIR_NM_SZ);
	de.inum = inum;

	int ret = writei(dp, (char *)&de, off, sizeof(struct dirent));
	if(ret != sizeof(struct dirent))
		panic("dir_link\n");
}

struct inode *idup(struct inode *ip)
{
	ip->ref++;
	return ip;
}

