#include "stdio.h"

int main(int argc, char *argv[])
{
	int seq = 0;
	while(seq < 5)
	{
		printf("tick %d\n", seq);
		sleep(1000);
		seq++;
	}
	return 0;
}

