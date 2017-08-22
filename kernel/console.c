#include "kernel.h"

extern struct CPU cpu;

struct _input {
	int r;
	int w;
	char buf[INPUT_BUFF];
}input;

void cprintfint(int data, int base, bool sign) 
{
	static char digitals[] = "0123456789ABCDEF";
	char buf[32] = {0};

	uint abs_data = 0;
	if(sign && (sign = data < 0))
		abs_data = -data;
	else
		abs_data = data;

	int i = 0;
	do {
		buf[i++] = digitals[abs_data % base];
	}while((abs_data /= base) != 0);

	if(base == 16) {
		buf[i++] = 'x';
		buf[i++] = '0';
	}
	if(sign)
		buf[i++] = '-';

	while(--i >= 0)
		console_putc(buf[i]);
}

void cprintfarg(char *fmt, uint *argp) 
{
	int c;
	//uint *argp;
	char *s;
	int i = 0;
	for(;(c = fmt[i]&0xff) != 0; i++) {
		if(c != '%'){
			console_putc(c);
			continue;
		}
		c = fmt[++i]&0xff;
		if(c == 0)
			break;
		switch(c) {
			case 's':
				if((s = (char *)*argp++) == 0){
					s= "(null)";
				}
				for(; *s != 0; s++) {
					console_putc(*s);
				}
			break;
			case 'd':
				cprintfint(*argp++, 10, true);
			break;
			case 'x':
			case 'p':
				cprintfint(*argp++, 16, false);
			break;
			case '%':
				console_putc('%');
			break;
			default:
				console_putc('%');
				console_putc(c);
			break;
		}
	}
}

void cprintf(char *fmt, ...) {
	uint *argp = (uint*)(&fmt + 1);
	cprintfarg(fmt, argp);
}

void panic(char *fmt, ...)
{
	cli();
	uint *argp = (uint*)(&fmt + 1);
	cprintf("MINIOS PANIC: ");
	cprintfarg(fmt, argp);
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

int console_read(struct inode *ip, char *dst, int len)
{
	int tar = len;
	if(input.r == input.w)
		sleep(&input.r);
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

int console_write(struct inode *ip, char *src, int len)
{
	int i;
	for(i = 0; i < len; i++) {
		console_putc(src[i]);
	}
	return 0;
}

void init_console()
{
	register_devrw(CONSOLE, console_read, console_write);
	enable_pic(IRQ_KBD);
	init_kdb();
}

void console_proc(int data)
{
	switch(data) {
	case '\b': 
		data = BACKSPACE;
		input.buf[input.w] = 0;
		input.w--;
		console_putc(data);
		break;
	case C('D'):
		cpu.cur_proc->killed = 1;
		wakeup(&input.r);
		break;
	default:
		data = (data == '\r' ? '\n' : data);
		if(input.w < INPUT_BUFF) {
			input.buf[input.w] = data;
			input.w++;
		}
		console_putc(data);
		if(data == '\n'){
			wakeup(&input.r);
		}
		break;
	}
}

