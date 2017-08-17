#include "kernel.h"

void init_kdb()
{
	wait_kbc_ready();
	outb(KEY_CMD, 0x60);
	wait_kbc_ready();
	outb(KEY_DATA, 0x47);
}

void wait_kbc_ready()
{
	while(1) {
		if((inb(KEY_STA) & KEYBOARD_READY) == 0)
			break;
	}
}

void kbd_proc()
{
	int data = inb(KEY_DATA);
	console_proc(data);
}

