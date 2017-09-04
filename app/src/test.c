#include "stdio.h"
#include "unistd.h"

void sig_handle(int sig)
{
	printf("signal is %d\n", sig);
	//exit();
}

int main(int argc, char *argv[])
{
	signal(SIG_INT, sig_handle);
	int fd = open("/dev/sys", 0);
	printf("fd is %d\n", fd);
	char sys[4096];
	read(fd, sys, 4096);
	printf(sys);
	return 0;
}

