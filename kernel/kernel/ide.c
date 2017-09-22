#include "kernel.h"

struct block_buf *ide_queue;

void init_ide()
{
  	enable_pic(IRQ_IDE);
  	ide_wait(false);

/*  	// Check if disk 1 is present
  	outb(0x1f6, 0xe0 | (1<<4));
  	for(i=0; i<1000; i++){
  		if(inb(0x1f7) != 0){
      		disk1 = true;
      		break;
    	}
  	}
*/
  	// Switch back to disk 0.
  	outb(0x1f6, 0xe0 | (0<<4));
}

// Wait for IDE disk to become ready.
int ide_wait(bool checkerr)
{
  int r;

  while(((r = inb(0x1f7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY)
    ;
  if(checkerr && (r & (IDE_DF|IDE_ERR)) != 0)
    return -1;
  return 0;
}

void ide_proc()
{
	struct block_buf *buf;
	if((buf = ide_queue) == NULL)
		return;
	ide_queue = ide_queue->qnext;
	if(!(buf->flags & B_DIRTY) && ide_wait(true) >= 0)
		insl(0x1f0, buf->data, BLOCK_SIZE / 4);
	buf->flags |= B_VALID;
	buf->flags &= ~B_DIRTY;
	wakeup_on(buf);
	ide_io();
}

void ide_rw(struct block_buf *buf)
{
	if((buf->flags & (B_DIRTY | B_VALID)) == B_VALID)
		panic("ide_io nothing to do\n");
	buf->qnext = NULL;
	struct block_buf *p = ide_queue;
	if(!p)
		ide_queue = buf;
	else {
		while(p->qnext)
			p = p->qnext;
		p->qnext = buf;
	}
	if(buf == ide_queue)
		ide_io();

	while((buf->flags & (B_DIRTY | B_VALID)) != B_VALID)
		sleep_on(buf);
}

void ide_io()
{
	struct block_buf *buf = ide_queue;
	if(!buf)
		return;
	int sector_per_block =  BLOCK_SIZE / SECTOR_SIZE;
	int sector = buf->sec * sector_per_block;

	ide_wait(false);
	outb(0x3f6, 0);  // generate interrupt
  	outb(0x1f2, sector_per_block);  // number of sectors
  	outb(0x1f3, sector & 0xff);
  	outb(0x1f4, (sector >> 8) & 0xff);
  	outb(0x1f5, (sector >> 16) & 0xff);
  	outb(0x1f6, 0xe0 | (buf->dev << 4) | ((sector >> 24) & 0x0f));
  	if(buf->flags & B_DIRTY){
  	  outb(0x1f7, sector_per_block == 1 ? IDE_CMD_WRITE : IDE_CMD_WRMUL);
  	  outsl(0x1f0, buf->data, BLOCK_SIZE / 4);
  	} else {
  	  outb(0x1f7, sector_per_block == 1 ? IDE_CMD_READ : IDE_CMD_RDMUL);
  	}
}

