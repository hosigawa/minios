#include "mmu.h"

.globl start
start:
	pushl $0
	movl $SYS_FORK, %eax
	int $T_SYSCALL
	cmp $0, %eax
	je procb

proca:
	pushl $stra
	pushl 0
	movl $SYS_PRINT, %eax
	int $T_SYSCALL
	popl %eax
	popl %eax
	jmp proca

procb:
	pushl $strb
	pushl 0
	movl $SYS_PRINT, %eax
	int $T_SYSCALL
	popl %eax
	popl %eax
	jmp procb

loop:
	jmp loop

stra:
	.string "THIS IS A\n\0"
strb:
	.string "THIS IS B\n\0"

