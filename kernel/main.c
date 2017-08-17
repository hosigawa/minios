#include "kernel.h"

#define MINIOS_VERSION  "0.0.1"
#define SUPER_PG_SIZE (4 * 1024 * 1024)

__attribute__((__aligned__(PG_SIZE)))
pde_t super_pgdir[1024] = {
  [0] = (0) | PTE_P | PTE_W | PTE_PS,
  [KERN_BASE >> 22] = (0) | PTE_P | PTE_W | PTE_PS,
};

extern char end[];

int main() 
{
	mem_init(end, P2V(SUPER_PG_SIZE));
	init_kvm();
	mem_init(P2V(SUPER_PG_SIZE), P2V(PHYSICAL_END));
	init_cpu();
	init_gdt();
	init_idt();
	init_pic();
	init_uart();
	init_timer();
	init_console();
	init_ide();
	user_init();

	sys_info("\nMINIOS ver.%s start...\n\n", MINIOS_VERSION);

	scheduler();
	return -1;
}

