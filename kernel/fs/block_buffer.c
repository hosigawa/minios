#include "kernel.h"

struct block_buf bufs[BLOCK_BUF_NUM];
struct block_buf *head;

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

