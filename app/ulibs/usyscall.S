#include "syscall.h"

#define SYS_CALL(name)\
	.globl name;\
	name:\
		movl $SYS_ ## name, %eax;\
		int $T_SYSCALL;\
		ret;

SYS_CALL(fork)
SYS_CALL(exec)
SYS_CALL(exit)
SYS_CALL(wait)
SYS_CALL(open)
SYS_CALL(close)
SYS_CALL(dup)
SYS_CALL(mknod)
SYS_CALL(read)
SYS_CALL(write)
SYS_CALL(fstat)
SYS_CALL(pwd)
SYS_CALL(ps)

