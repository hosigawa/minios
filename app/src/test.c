#include "stdio.h"
#include "unistd.h"

int main(int argc, char *argv[])
{
	int i = 0;
	while(i < 5) {
		printf("sleep %d\n", i);
		sleep(1);
		i++;
	}
	return 0;
}

