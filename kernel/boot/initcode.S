#include "syscall.h"

.globl start
start:
	pushl $argv
	pushl $init
	pushl $0
	movl $SYS_exec, %eax
	int $T_SYSCALL

exit:
	movl $SYS_exit, %eax;
	int $T_SYSCALL

init:
	.string "/init\0"

argv:
	.long init
	.long 0

