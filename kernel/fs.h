#ifndef __FS_H__
#define __FS_H__

#define BLOCK_BUF_NUM 30
#define BLOCK_SIZE 512
#define B_VALID 0x2  // buffer has been read from disk
#define B_DIRTY 0x4  // buffer needs to be written to disk

struct block_buf {
	int flags;
	int dev;
	int sec;
	struct block_buf *prev;
	struct block_buf *next;
	struct block_buf *qnext;
	char data[BLOCK_SIZE];
};

struct dinode {
	
};

void binit();
struct block_buf *bread(int dev, int sec);
void bwrite(int dev, int sec);
struct block_buf *bget(int dev, int sec);
void brelse(struct block_buf *buf);

#endif

