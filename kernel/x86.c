#include "kernel.h"

struct CPU cpu;

void init_cpu() 
{
	cpu.ncli = 0;
	cpu.intble = false;
	cpu.cur_proc = NULL;
	cpu.context = NULL;
}

void pushcli()
{
	uint eflags = read_eflags();
	cli();
	if(cpu.ncli++ == 0)
		cpu.intble = eflags & FL_IF;
}

void popsti()
{
	if(--cpu.ncli < 0)
		panic("popsti\n");
	if(cpu.ncli == 0 && cpu.intble)
		sti();
}

