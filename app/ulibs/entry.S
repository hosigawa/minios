#include "syscall.h"

.globl _start
_start:
	call main

	movl $SYS_exit, %eax;
	int $T_SYSCALL
error:
	loop error

