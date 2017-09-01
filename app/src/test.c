#include "stdio.h"
#include "unistd.h"

void sig_handle(int sig)
{
	printf("signal is %d\n", sig);
	exit();
}

int main(int argc, char *argv[])
{
	signal(9, sig_handle);
	char *p = NULL;
	printf(p);
	return 0;
}

