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

	int ret = 0;
	struct dirent de;
	char name[64];
	if((fd = open("/proc", 0)) > 0) {
		while((ret = read(fd, (char *)&de, sizeof(de))) == sizeof(de)) {
			if(de.inum == 0)
				continue;
			memset(name, 0, 64);
			memmove(name, "/proc/", 6);
			memmove(name + 6, de.name, strlen(de.name));
			unlink(name);
		}
		unlink("/proc");
		close(fd);
	}
	mkdir("/proc");
	mknod("/proc/1", 2, 1);

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

