#include "kernel.h"

uint _version = 0;

int main() 
{
	_version = MAKE_VER(0, 2, 0);
	init_uart();
	mem_init();
	init_kvm();
	init_cpu();
	init_gdt();
	init_idt();
	init_pic();
	init_timer();
	init_fs();
	init_ide();
	init_console();
	user_init();

	sys_info("\nMINIOS ver.%d.%d.%d start...\n\n", GET_VER_H(_version), GET_VER_M(_version), GET_VER_L(_version));

	scheduler();
}

