#include "crt.h"

int main()
{
	printf("init...\n");
	char *argv[] = {"/sh", 0};
	int pid, wtpid;
	while(1) {
		pid = fork();
		if(pid == 0) {
			exec("/sh", argv);
			printf("exec sh failre\n");
			return 0;
		}
		while((wtpid = wait()) > 0);
		printf("pid:%d exit error\n", wtpid);
	}

	return 0;
}

