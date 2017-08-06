#include "kernel.h"

#define MINIOS_VERSION  "0.0.1"

__attribute__((__aligned__(PG_SIZE)))
pde_t super_pgdir[1024] = {
  [0] = (0) | PTE_P | PTE_W | PTE_PS,
  [KERN_BASE >> 22] = (0) | PTE_P | PTE_W | PTE_PS,
};

extern char end[];

int main() 
{
	init_pic();
	init_uart();
	mem_init(end, P2V(4*1024*1024));
	init_kvm();
	init_gdt();
	init_idt();
	init_timer();
	mem_init(P2V(4*1024*1024), P2V(PHYSICAL_END));
	user_init();

	sys_info("\nMINIOS ver.%s start...\n\n", MINIOS_VERSION);

	scheduler();

	sys_info("kernal go on ...\n");

	for(;;);
}

