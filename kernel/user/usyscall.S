#include "mmu.h"

#define SYS_CALL(name)\
	.globl name;\
	name:\
		movl $SYS_ ## name, %eax;\
		int $T_SYSCALL;\
		ret;

SYS_CALL(print);

