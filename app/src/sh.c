#include "crt.h"

int main() 
{
	printf("init sh...\n");
	char *argv[] = {"ps", 0};
	int pid = fork();
	if(pid == 0) {
		exec("ps", argv);
	}

	int wtpid;
	while((wtpid = wait()) > 0);
	printf("sh: pid %d exit error\n", wtpid);
	return 0;
}

