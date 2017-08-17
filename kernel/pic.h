#ifndef __PIC_H__
#define __PIC_H__

#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW1		0x0020
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021

#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW1		0x00a0
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

#define T_IRQ0 32

#define IRQ_TIMER 0
#define IRQ_KBD 1
#define IRQ_COM1 4
#define IRQ_IDE 14

#define GET_IRQ(a) (T_IRQ0 + (a))

void init_pic();
void enable_pic(int irq);
void set_pic_mask(); 

#endif

