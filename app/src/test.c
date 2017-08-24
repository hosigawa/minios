#include "stdio.h"
#include "malloc.h"

int main(int argc, char *argv[])
{
	printf("malloc size free:%d, totol:%d, sys:%d\n", memory_size(false), memory_size(true), ps(NULL, 1));
	char *p1, *p2, *p3, *p4, *p5;
	p1 = (char *)malloc(4080);
	printf("malloc size free:%d, totol:%d, sys:%d\n", memory_size(false), memory_size(true), ps(NULL, 1));
	p2 = (char *)malloc(4080);
	printf("malloc size free:%d, totol:%d, sys:%d\n", memory_size(false), memory_size(true), ps(NULL, 1));
	printf("malloc size free:%d, totol:%d, sys:%d\n", memory_size(false), memory_size(true), ps(NULL, 1));
	p3 = (char *)malloc(4567);
	printf("malloc size free:%d, totol:%d, sys:%d\n", memory_size(false), memory_size(true), ps(NULL, 1));
	free(p2);
	printf("malloc size free:%d, totol:%d, sys:%d\n", memory_size(false), memory_size(true), ps(NULL, 1));
	free(p3);
	printf("malloc size free:%d, totol:%d, sys:%d\n", memory_size(false), memory_size(true), ps(NULL, 1));
	p4 = malloc(408032);
	printf("malloc size free:%d, totol:%d, sys:%d\n", memory_size(false), memory_size(true), ps(NULL, 1));
	p5 = malloc(4080);
	printf("malloc size free:%d, totol:%d, sys:%d\n", memory_size(false), memory_size(true), ps(NULL, 1));
	free(p5);
	free(p1);
	printf("malloc size free:%d, totol:%d, sys:%d\n", memory_size(false), memory_size(true), ps(NULL, 1));
	free(p4);
	printf("malloc size free:%d, totol:%d, sys:%d\n", memory_size(false), memory_size(true), ps(NULL, 1));
	return 0;
}

