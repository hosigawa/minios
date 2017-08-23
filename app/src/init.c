#include "stdio.h"

int main(int argc, char **argv)
{
	int fd;
	if((fd = open("/dev/console", 0)) < 0) {
		mknod("/dev/console", 1, 1);
		fd = open("/dev/console", 0);
	}
	dup(fd);
	dup(fd);

	char *sh_argv[] = {"/bin/sh", 0};
	int pid, wtpid;
	while(1) {
		pid = fork();
		if(pid == 0) {
			exec(sh_argv[0], sh_argv);
			printf("exec sh failre\n");
			return 0;
		}
		while((wtpid = wait()) > 0) {
			if(wtpid == pid)
				break;
		}
	}
	printf("init exit error\n");

	return 0;
}

