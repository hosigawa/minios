#include "stdio.h"
#include "unistd.h"

int main(int argc, char **argv)
{
	struct time_v tm;
	gettimeofday(&tm);
	printf("%d-%02d-%02d %02d:%02d:%02d\n", tm.year,tm.mon,tm.day,tm.h,tm.m,tm.s);
	return 0;
}

