#include "kernel.h"

struct block_buf *ide_queue;
bool disk1 = false;

void init_ide()
{
	int i;

  	enable_pic(IRQ_IDE);
  	ide_wait(false);

  	// Check if disk 1 is present
  	outb(0x1f6, 0xe0 | (1<<4));
  	for(i=0; i<1000; i++){
  		if(inb(0x1f7) != 0){
      		disk1 = true;
			sys_info("disk1 found\n");
      		break;
    	}
  	}

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
		insl(0x1f0, buf->data, 128);
	buf->flags |= B_VALID;
	buf->flags &= ~B_DIRTY;
	wakeup(buf);
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
		p->next = buf;
	}
	if(buf == ide_queue)
		ide_io();

	while((buf->flags & (B_DIRTY | B_VALID)) != B_VALID)
		sleep(buf);
}

void ide_io()
{
	struct block_buf *buf = ide_queue;
	if(!buf)
		return;
	ide_queue = ide_queue->qnext;
	ide_wait(false);
	outb(0x3f6, 0);  // generate interrupt
  	outb(0x1f2, 1);  // number of sectors
  	outb(0x1f3, buf->sec & 0xff);
  	outb(0x1f4, (buf->sec >> 8) & 0xff);
  	outb(0x1f5, (buf->sec >> 16) & 0xff);
  	outb(0x1f6, 0xe0 | ((buf->dev & 1)<<4) | ((buf->sec >> 24) & 0x0f));
  	if(buf->flags & B_DIRTY){
  	  outb(0x1f7, IDE_CMD_WRITE);
  	  outsl(0x1f0, buf->data, 128);
  	} else {
  	  outb(0x1f7, IDE_CMD_READ);
  	}
}

