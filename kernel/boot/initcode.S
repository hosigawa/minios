#include "syscall.h"

.globl start
start:
	pushl $envp
	pushl $argv
	pushl $init
	pushl $0
	movl $SYS_execv, %eax
	int $T_SYSCALL

exit:
	movl $SYS_exit, %eax;
	int $T_SYSCALL

init:
	.string "/bin/init\0"
env:
	.string "/bin\0"

argv:
	.long init
	.long 0

envp:
	.long env
	.long 0

