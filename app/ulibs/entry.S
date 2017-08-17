#include "syscall.h"

.globl _start
_start:
	call main

	pushl %eax
	pushl 0
	movl $SYS_exit, %eax;
	int $T_SYSCALL
error:
	loop error

