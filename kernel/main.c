#include "kernel.h"

#define MINIOS_VERSION  "0.0.1"

int main() 
{
	init_uart();
	mem_init();
	init_kvm();
	init_cpu();
	init_gdt();
	init_idt();
	init_pic();
	init_timer();
	init_file();
	init_ide();
	init_console();
	init_procfs();
	user_init();

	sys_info("\nMINIOS ver.%s start...\n\n", MINIOS_VERSION);

	scheduler();
}

