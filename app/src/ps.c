#include "stdio.h"
#include "malloc.h"

int main()
{
	char proc_info[4096];
	memset(proc_info, 0, 4096);
	int fd = open("/dev/proc", 0);
	if(fd < 0) {
		printf("ps: open /dev/proc error\n");
		return -1;
	}
	read(fd, proc_info, 4096);
	printf(" PID PPID   VSZ    STAT       CMD\n");
	printf(proc_info);
	close(fd);
	return 0;
}

