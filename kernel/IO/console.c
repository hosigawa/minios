#include "kernel.h"

extern struct CPU cpu;

struct _input {
	int r;
	int w;
	char buf[INPUT_BUFF];
}input;

static void console_cputc(char *dst, int data)
{
	console_putc(data);
}

void cprintf(char *fmt, ...) {
	uint *argp = (uint*)(&fmt + 1);
	vprintf(fmt, argp, NULL, console_cputc);
}

void panic(char *fmt, ...)
{
	cli();
	uint *argp = (uint*)(&fmt + 1);
	cprintf("MINIOS PANIC: ");
	vprintf(fmt, argp, NULL, console_cputc);
	for(;;);
}

void console_putc(int c) 
{
	cga_putc(c);
	if(c == BACKSPACE){
    	uart_putc('\b'); 
		uart_putc(' '); 
		uart_putc('\b');
	}
	else
		uart_putc(c);
}

int console_read(struct inode *ip, char *dst, int off, int len)
{
	int tar = len;
	if(input.r == input.w)
		sleep_on(&input.r);
	for(; input.r < input.w; input.r++) {
		int c = input.buf[input.r];
		*dst++ = c;
		if(--len <= 0)
			break;
		if(c == '\n')
			break;
	}
	input.w = 0;
	input.r = 0;
	return tar - len;
}

int console_write(struct inode *ip, char *src, int off, int len)
{
	int i;
	for(i = 0; i < len; i++) {
		console_putc(src[i]);
	}
	return 0;
}

void init_console()
{
	enable_pic(IRQ_KBD);
	init_kdb();
}

bool udrl = false;
void proc_udrl(int data) {
	switch(data) {
		case '\x5b':
			return;
		case '\x41':
			break;
		case '\x42':
			break;
		case '\x43':
			/*console_putc('\x1b');
			console_putc('\x5b');
			console_putc('\x43');*/
			break;
		case '\x44':
			/*console_putc('\x1b');
			console_putc('\x5b');
			console_putc('\x44');*/
			break;
	}
	udrl = false;
}

void console_proc(int (*getc)(void))
{
	int data;
	while((data = getc()) > 0) {
		if(udrl){
			proc_udrl(data);
			continue;
		}
		switch(data) {
			case '\x1b':
				udrl = true;
				break;
			case '\b': 
				if(input.w == 0)
					break;
				data = BACKSPACE;
				input.buf[input.w] = 0;
				input.w--;
				console_putc(data);
				break;
			case C('C'):
			case C('D'):
				if(cpu.cur_proc)
					cpu.cur_proc->killed = 1;
				wakeup_on(&input.r);
				break;
			default:
				data = (data == '\r' ? '\n' : data);
				if(input.w < INPUT_BUFF) {
					input.buf[input.w] = data;
					input.w++;
				}
				console_putc(data);
				if(data == '\n'){
					wakeup_on(&input.r);
				}
				break;
		}
	}
}

