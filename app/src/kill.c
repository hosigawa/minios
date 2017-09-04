#include "stdio.h"
#include "unistd.h"

int main(int argc, char *argv[])
{
	int sig = atoi(argv[1]);
	int pid = atoi(argv[2]);
	int ret = kill(pid, sig);
	if(ret < 0) {
		printf("kill: %d: process not exist\n", pid);
		return -1;
	}
	printf("kill: pid:%d, sig:%d\n", pid, sig);
	return 0;
}

