#include "stdio.h"
#include "malloc.h"

char *STATUS[] = {
	"UNUSED  ", 
	"EMBRYO  ",
	"READY   ",
	"RUNNING ",
	"SLEEPING",
	"ZOMBIE  ",
};

int main()
{
	printf("ps\n");
	return 0;
}

