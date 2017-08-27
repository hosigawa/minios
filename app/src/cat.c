#include "stdio.h"
#include "malloc.h"

int main(int argc, char *argv[]) 
{
	struct file_stat stat;
	int fd;
	if(argc < 2)
		fd = open("/dev/console", 0);
	else 
		fd = open(argv[1], 0);
	if(fd < 0) {
		printf("cat: %s: No such file or directory\n", argv[1]);
		return -1;
	}
	if(fstat(fd, &stat) < 0){
		printf("cat: %s: get file stat error\n", argv[1]);
		close(fd);
		return -1;
	}
	if(stat.type == T_DIR) {
		printf("cat: %s: Is a directory\n", argv[1]);
		close(fd);
		return -1;
	}

	if(stat.type == T_DEV) {
		char buf[4096] = {0};
		read(fd, buf, 4096);
		char *p = buf;
		while(*p) {
			write(stdout, p++, 1);
		}
		close(fd);
		return 0;
	}

	char *memblock = malloc(stat.size);
	if(!memblock) {
		printf("cat: %s: memory not enough\n");
		close(fd);
		return -1;
	}
	read(fd, memblock, stat.size);

	int i;
	for(i = 0; i < stat.size; i++) {
		write(stdout, memblock + i, 1);
	}
	free(memblock);
	close(fd);
	return 0;
}

