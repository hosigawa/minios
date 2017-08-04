#include "mmu.h"

.globl _start
_start = V2P_NC(entry)

.globl entry
entry:
	movl %cr4, %eax
	orl $(CR4_PSE), %eax
	movl %eax, %cr4

	movl $(V2P_NC(super_pgdir)), %eax
	movl %eax, %cr3

	movl %cr0, %eax
	orl $(CR0_PG | CR0_WP), %eax
	movl %eax, %cr0

	movl $(stack + KSTACKSIZE), %esp

	mov $main, %eax
	jmp *%eax

.comm stack, KSTACKSIZE

