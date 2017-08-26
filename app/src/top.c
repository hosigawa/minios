#include "stdio.h"
#include "malloc.h"

int main()
{
	char sys_info[4096];
	memset(sys_info, 0, 4096);
	int fd = open("/dev/sys", 0);
	if(fd < 0) {
		printf("ps: open /dev/sys error\n");
		return -1;
	}
	read(fd, sys_info, 4096);
	printf(sys_info);
	close(fd);
	return 0;
}

