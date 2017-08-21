#include "libc.h"

int main(int argc, char **argv)
{
	int fd;
	if((fd = open("/console", 0)) < 0) {
		mknod("/console", 1, 1);
		fd = open("/console", 0);
	}
	dup(fd);
	dup(fd);

	char *sh_argv[] = {"/sh", "aa", "bb", 0};
	int pid, wtpid;
	while(1) {
		pid = fork();
		if(pid == 0) {
			exec("/sh", sh_argv);
			printf("exec sh failre\n");
			return 0;
		}
		while((wtpid = wait()) > 0);
		printf("pid:%d exit error\n", wtpid);
	}

	return 0;
}

