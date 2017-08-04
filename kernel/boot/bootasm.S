#include "asm.h"

#define SEG_KCODE  1
#define SEG_KDATA  2

#define CR0_PE  1

.code16
.globl start

start:
	cli
	xorw %ax, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss

#open line A20
seta20.1:
	inb $0x64, %al
	testb $0x2, %al
	jnz seta20.1

	movb $0xd1, %al
	outb %al, $0x64

seta20.2:
	inb $0x64, %al
	testb $0x2, %al
	jnz seta20.2

	movb $0xdf, %al
	outb %al, $0x60
	
	lgdt gdtdesc
	movl %cr0, %eax
	orl $CR0_PE, %eax
	movl %eax, %cr0

	ljmp $(SEG_KCODE << 3), $start32

.code32
start32:
	movw $(SEG_KDATA << 3), %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss

	movw $0, %ax
	movw %ax, %fs
	movw %ax, %gs

	movl $start, %esp
	call bootmain

spin:
	jmp spin

.p2align 2
gdt:
	SEG_NULLASM
	SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)
	SEG_ASM(STA_W, 0x0, 0xffffffff)

gdtdesc:
	.word (gdtdesc - gdt -1)
	.long gdt

