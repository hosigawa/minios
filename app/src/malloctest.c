#include "stdio.h"
#include "malloc.h"

int main(int argc, char *argv[])
{
	printf("malloc size free:%d, totol:%d\n", memory_size(false), memory_size(true));
	char *p1, *p2, *p3, *p4, *p5;
	p1 = (char *)malloc(40801237);
	printf("malloc size free:%d, totol:%d\n", memory_size(false), memory_size(true));
	p2 = (char *)malloc(4080);
	printf("malloc size free:%d, totol:%d\n", memory_size(false), memory_size(true));
	free(p1);
	printf("malloc size free:%d, totol:%d\n", memory_size(false), memory_size(true));
	p3 = (char *)malloc(4567);
	printf("malloc size free:%d, totol:%d\n", memory_size(false), memory_size(true));
	free(p2);
	printf("malloc size free:%d, totol:%d\n", memory_size(false), memory_size(true));
	free(p3);
	printf("malloc size free:%d, totol:%d\n", memory_size(false), memory_size(true));
	p4 = malloc(408032);
	printf("malloc size free:%d, totol:%d\n", memory_size(false), memory_size(true));
	p5 = malloc(4080);
	printf("malloc size free:%d, totol:%d\n", memory_size(false), memory_size(true));
	free(p5);
	printf("malloc size free:%d, totol:%d\n", memory_size(false), memory_size(true));
	free(p4);
	printf("malloc size free:%d, totol:%d\n", memory_size(false), memory_size(true));
	return 0;
}

