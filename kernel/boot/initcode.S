#include "mmu.h"

.globl start
start:
	pushl $argv
	pushl $init
	pushl $0
	movl $SYS_exec, %eax
	int $T_SYSCALL

loop:
	jmp loop

init:
	.string "/init\0"

argv:
	.long 0

