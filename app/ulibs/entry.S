#include "syscall.h"

.globl _start
_start:
	call crt_init
	call main
	call crt_fini

	pushl %eax
	pushl $0
	movl $SYS_exit, %eax;
	int $T_SYSCALL
error:
	loop error

