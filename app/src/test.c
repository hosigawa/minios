#include "stdio.h"
#include "unistd.h"

int main(int argc, char *argv[])
{
	char *p = NULL;
	printf(p);
	uint tm = 0;
	localtime(&tm);
	printf("unixstamp is %u\n", tm);
	return 0;
}

