#ifndef __X86_H__
#define __X86_H__

#include "type.h"

static inline unsigned char inb(unsigned short port) 
{
	unsigned char data;
	asm volatile (
		"in %1, %0":
		"=a"(data):
		"d"(port)
	);
	return data;
}

static inline void outb(unsigned short port, unsigned char data) 
{
	asm volatile(
		"outb %0, %1"::
		"a"(data), "d"(port)
	);
}

static inline void insl(int port, void *addr, int cnt) 
{
	asm volatile(
		"cld; rep insl" :
		"=D" (addr), "=c" (cnt) :
		"d" (port), "0" (addr), "1" (cnt) :
		"memory", "cc"
	);
}

static inline void stosb(void *addr, int data, int size) 
{
	asm volatile(
		"cld; rep stosb":
		"=D"(addr), "=c"(size):
		"0"(addr), "1"(size), "a"(data):
		"memory", "cc"
	);
}

struct gdt_desc;

static inline void lgdt(struct gdt_desc *gdt, int size) 
{
	volatile ushort pd[3];

	pd[0] = size-1;
	pd[1] = (uint)gdt;
	pd[2] = (uint)gdt >> 16;

	asm volatile("lgdt (%0)" : : "r" (pd));
}

struct gate_desc;

static inline void lidt(struct gate_desc *idt, int size)
{
  volatile ushort pd[3];

  pd[0] = size-1;
  pd[1] = (uint)idt;
  pd[2] = (uint)idt >> 16;

  asm volatile("lidt (%0)" : : "r" (pd));
}

static inline void lcr3(uint val)
{
  asm volatile("movl %0,%%cr3" : : "r" (val));
}

static inline void cli()
{
	asm volatile("cli");
}

static inline void sti()
{
	asm volatile("sti");
}

static inline void ltr(ushort sel)
{
  asm volatile("ltr %0" : : "r" (sel));
}

#endif

