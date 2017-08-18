#include "libc.h"

int main()
{
	int fd;
	if((fd = open("/console", 0)) < 0) {
		mknod("/console", 1, 1);
		fd = open("/console", 0);
	}
	dup(fd);
	dup(fd);

	printf("init...\n");
	char *argv[] = {"/sh", 0};
	int pid, wtpid;
	while(1) {
		pid = fork();
		if(pid == 0) {
			exec("/sh", argv);
			printf("exec sh failre\n");
			return 0;
		}
		while((wtpid = wait()) > 0);
		printf("pid:%d exit error\n", wtpid);
	}

	return 0;
}

