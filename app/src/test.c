#include "stdio.h"
#include "unistd.h"

int main(int argc, char *argv[])
{
	srand(0);
	int ran = rand() % 100;
	printf("ran:%d\n", ran);
	if(ran % 2 == 0){
		printf("You are female\n");
	}
	else {
		printf("You are male\n");
	}
	return 0;
}

