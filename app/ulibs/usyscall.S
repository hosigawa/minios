#include "syscall.h"

#define SYS_CALL(name)\
	.globl name;\
	name:\
		movl $SYS_ ## name, %eax;\
		int $T_SYSCALL;\
		ret;

SYS_CALL(vprintf);
SYS_CALL(fork);
SYS_CALL(exec);
SYS_CALL(exit);
SYS_CALL(wait);
SYS_CALL(ps);

