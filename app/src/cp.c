#include "stdio.h"

int main(int argc, char *argv[])
{
	if(argc < 2) {
		printf("cp: missing file operand\n");
		return -1;
	}
	if(argc < 3) {
		printf("cp: missing destination file operand after '%s'\n", argv[1]);
		return -1;
	}
	int src, dst;
	struct file_stat stat;
	src = open(argv[1], 0);
	if(!src) {
		printf("cp: cannot stat '%s': No such file or directory\n", argv[1]);
		return -1;
	}
	fstat(src, &stat);
	if(stat.type == T_DEV) {
		printf("cp: '%s': cannot copy device file\n", argv[1]);
		close(src);
		return -1;
	}

	dst = open(argv[2], 0);
	if(dst > 0) {
		printf("cp: '%s' exist\n", argv[2]);
		close(src);
		close(dst);
		return -1;
	}

	dst = open(argv[2], O_CREATE);
	if(!dst) {
		printf("cp: create '%s' failed\n", argv[2]);
		close(src);
		return -1;
	}

	char *mem = malloc(stat.size);
	if(!mem) {
		printf("cp: malloc failed\n");
		close(src);
		close(dst);
		return -1;
	}
	read(src, mem, stat.size);
	write(dst, mem, stat.size);
	free(mem);
	close(src);
	close(dst);

	return 0;
}

