#ifndef __IDE_H__
#define __IDE_H__

#include "bio.h"

#define SECTOR_SIZE   512
#define IDE_BSY       0x80
#define IDE_DRDY      0x40
#define IDE_DF        0x20
#define IDE_ERR       0x01

#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30
#define IDE_CMD_RDMUL 0xc4
#define IDE_CMD_WRMUL 0xc5

void init_ide();
int ide_wait(bool checkerr);
void ide_proc();
void ide_rw(struct block_buf *buf);
void ide_io();

#endif

