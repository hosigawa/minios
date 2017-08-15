#include "usyscall.h"

int main()
{
	char *argv[] = {"/sh", 0};
	int pid, wtpid;
	while(1) {
		pid = fork();
		if(pid == 0) {
			exec("/sh", argv);
		}
	
		while((wtpid = wait() > 0));
			print("init wait error\n");
	}

	return 0;
}

