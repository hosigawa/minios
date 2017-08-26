#include "kernel.h"

void init_procfs()
{
}

int proc_read(struct inode *ip, char *dst, int len)
{
	printf("proc read\n");
	return 0;
}

int proc_write(struct inode *ip, char *dst, int len)
{
	printf("proc write\n");
	return 0;
}

