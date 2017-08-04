#include "kernel.h"

static ushort irq_mask = 0xFFFF & ~(1<<2);

void init_pic()
{
	outb(PIC0_IMR, 0xff);
	outb(PIC1_IMR, 0xff);
	
	outb(PIC0_ICW1, 0x11);
	outb(PIC0_ICW2, 0x20);
	outb(PIC0_ICW3, 1 << 2);
	outb(PIC0_ICW4, 0x03);
	
	outb(PIC1_ICW1, 0x11);
	outb(PIC1_ICW2, 0x28);
	outb(PIC1_ICW3, 0x2);
	outb(PIC1_ICW4, 0x03);

	set_pic_mask();
	
	//sys_info("PIC INIT\n");
}

void enable_pic(int irq) 
{
	irq_mask &= ~(1<<irq);
	set_pic_mask();
}

void set_pic_mask()
{
	outb(PIC0_IMR, irq_mask);
	outb(PIC1_IMR, irq_mask >> 8);
}

