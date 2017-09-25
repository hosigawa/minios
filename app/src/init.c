#include "stdio.h"
#include "fs.h"

void init_dev(char *path, int major, int minor)
{
	int fd;
	if((fd = open(path, 0)) < 0) {
		mknod(path, major, minor);
	}
	else {
		close(fd);
	}
}

void init_devices()
{
	int fd;
	if((fd = open("/dev/console", 0)) < 0) {
		mknod("/dev/console", DEV_CONSOLE, 1);
		fd = open("/dev/console", 0);
	}
	dup(fd);
	dup(fd);

	init_dev("dev/null", DEV_NULL, 1);
}

int main(int argc, char **argv)
{
	init_devices();
	char *sh_argv[] = {"/bin/sh", 0};
	char *sh_env[] = {"/bin", 0};
	int pid, wtpid;
	while(1) {
		pid = fork();
		if(pid == 0) {
			execv(sh_argv[0], sh_argv, sh_env);
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

