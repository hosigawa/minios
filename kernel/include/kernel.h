#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "list.h"
#include "type.h"
#include "x86.h"
#include "param.h"
#include "console.h"
#include "libc.h"
#include "memory.h"
#include "vm.h"
#include "trap.h"
#include "pic.h"
#include "uart.h"
#include "cga.h"
#include "mmu.h"
#include "timer.h"
#include "proc.h"
#include "syscall.h"
#include "sysproc.h"
#include "ide.h"
#include "fs.h"
#include "elf.h"
#include "signal.h"

#define LOG cprintf
#define sys_info cprintf
#define err_info cprintf
#define printf cprintf

#define MAKE_VER(h,m,l) (((h)<<16) + ((m)<<8) + (l))
#define GET_VER_H(v) (((v)>>16)&0xff)
#define GET_VER_M(v) (((v)>>8)&0xff)
#define GET_VER_L(v) ((v)&0xff)

#define SIZEOF_STRUCT(ss) (sizeof(ss) / sizeof(ss[0]))

#endif

