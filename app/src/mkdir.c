#include "stdio.h"

int main(int argc, char* argv[])
{
	if(argc < 2) {
		printf("mkdir: missing operand\n");
		return -1;
	}
	if(mkdir(argv[1]) < 0) 
		printf("mkdir %s error\n", argv[1]);
	return 0;
}

