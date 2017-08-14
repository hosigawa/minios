#include "usyscall.h"

int main()
{
	print("run in c code here ...\n");
	int pid = fork();
	char *argv[] = {0};
	if(pid == 0) {
		exec("/sh", argv);
	}
	else {
		for(;;)
			print("return to parent\n");
	}
	for(;;);
	return 0;
}

