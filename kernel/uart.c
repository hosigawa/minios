#include "kernel.h"

static bool buart = false;

void init_uart()
{
	// Turn off the FIFO
	outb(COM1+2, 0);
  
	// 9600 baud, 8 data bits, 1 stop bit, parity off.
	outb(COM1+3, 0x80);    // Unlock divisor
	outb(COM1+0, 115200/9600);
	outb(COM1+1, 0);
 	outb(COM1+3, 0x03);    // Lock divisor, 8 data bits.
  	outb(COM1+4, 0);
  	outb(COM1+1, 0x01);    // Enable receive interrupts.

  	// If status is 0xFF, no serial port.
  	if(inb(COM1+5) == 0xFF)
  	  	return;
  	buart = true;

  	// Acknowledge pre-existing interrupt conditions;
  	// enable interrupts.
  	inb(COM1+2);
  	inb(COM1+0);
  	enable_pic(IRQ_COM1);
  
	//char *p;
  	// Announce that we're here.
  	//for(p="uart init...\n"; *p; p++)
    //	uart_putc(*p);
}

void uart_putc(int c)
{
  	if(!buart)
    	return;
  	outb(COM1+0, c);
}

void uart_proc()
{
	if(!buart)
		return;
	int data = uart_getc();
	console_proc(data);
}

int uart_getc()
{
  if(!buart)
    return -1;
  if(!(inb(COM1+5) & 0x01))
    return -1;
  return inb(COM1+0);
}

