#include "stdio.h"
#include "unistd.h"

void sig_handle(int sig)
{
	printf("signal is %d\n", sig);
	exit();
}

int main(int argc, char *argv[])
{
	signal(SIG_INT, sig_handle);
	char *p = NULL;
	*p = 0;
	return 0;
}

