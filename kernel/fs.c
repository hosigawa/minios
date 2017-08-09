#include "kernel.h"

struct block_buf bufs[BLOCK_BUF_NUM];
struct block_buf *head;

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
	struct block_buf *p = head;
	while(p->next != head){
		if(p->dev == dev && p->sec == sec) {
			buf = p;
			break;
		}
		p = p->next;
	}

	if(!buf) {
		buf = head->prev;
		buf->flags = 0;
		buf->dev = dev;
		buf->sec = sec;
	}
	return buf;
}

void brelse(struct block_buf *buf)
{
	head->next->prev = buf;
	buf->next = head->next;
	head->next = buf;
	buf->prev = head;
}

