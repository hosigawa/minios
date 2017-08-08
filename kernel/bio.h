#ifndef __BIO_H__
#define __BIO_H__

#define BLOCK_SIZE 512
#define B_VALID 0x2  // buffer has been read from disk
#define B_DIRTY 0x4  // buffer needs to be written to disk

struct block_buf {
	int flags;
	int device;
	int sector;
	struct block_buf *prev;
	struct block_buf *next;
	struct block_buf *qnext;
	char data[BLOCK_SIZE];
};

#endif

