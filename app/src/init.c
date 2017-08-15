#include "usyscall.h"

int main()
{
	while(1) {
		print("run in c code here ...\n");
		char *argv[] = {0};
		int pid = fork();
		if(pid == 0) {
			print("ready to exec /sh\n");
			exec("/sh", argv);
		}
		else {
			print("return to parent\n");
		}
		
		int wtpid;
		while((wtpid = wait() > 0));
		print("init error\n");
	}

	return 0;
}

