#ifndef __UART_H__
#define __UART_H__

#define COM1 0x3f8

void init_uart();
void uart_putc(int c);
void uart_proc();
int uart_getc();

#endif

