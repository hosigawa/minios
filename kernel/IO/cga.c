#include "kernel.h"

#define CGA_CTRL_PORT 0x03d4
#define CGA_DATA_PORT 0x03d5
#define CGA_MODE 0x0700
#define BACKSPACE 0x100
#define MAX_ROW 25
#define MAX_COL 80

static ushort *cga_data = (ushort *)P2V(0xb8000);

void cga_putc(int c) 
{
	int pos = 0;

	outb(CGA_CTRL_PORT, 14);
	pos = inb(CGA_DATA_PORT) << 8;
	outb(CGA_CTRL_PORT, 15);
	pos |= inb(CGA_DATA_PORT);

	if(c == '\n') {
		pos += MAX_COL - pos%MAX_COL;
	}
	else if (c == BACKSPACE) {
		if(pos > 0) {
			cga_data[--pos] = ' ' | CGA_MODE;
		}
	}
	else {
		cga_data[pos++] = (c & 0xff) | CGA_MODE;
	}

	if(pos/MAX_COL >= (MAX_ROW - 1)) {
		memmove(cga_data, cga_data + MAX_COL, sizeof(ushort)*MAX_COL*(MAX_ROW - 2));
		pos -= MAX_COL;
		memset(cga_data + pos, 0, sizeof(ushort)*((MAX_ROW - 1)*MAX_COL - pos));
	}

	outb(CGA_CTRL_PORT, 14);
	outb(CGA_DATA_PORT, pos >> 8);
	outb(CGA_CTRL_PORT, 15);
	outb(CGA_DATA_PORT, pos & 0xff);
	cga_data[pos] = ' ' | 0x0700;
}

