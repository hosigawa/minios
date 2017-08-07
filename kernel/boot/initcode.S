#include "mmu.h"

.globl start
start:
	movl $SYS_PRINT, %eax
	int $T_SYSCALL
loop:
	jmp loop
