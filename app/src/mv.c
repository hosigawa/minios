#include "stdio.h"

int main(int argc, char *argv[])
{
	if(argc < 2) {
		printf("mv: missing file operand\n");
		return -1;
	}
	if(argc < 3) {
		printf("mv: missing destination file operand after '%s'\n", argv[1]);
		return -1;
	}
	int src, dst;
	struct file_stat stat;
	src = open(argv[1], 0);
	if(!src) {
		printf("mv: cannot stat '%s': No such file or directory\n", argv[1]);
		return -1;
	}
	fstat(src, &stat);
	if(stat.type == T_DEV) {
		printf("mv: '%s': cannot copy device file\n", argv[1]);
		close(src);
		return -1;
	}

	dst = open(argv[2], 0);
	if(dst > 0) {
		printf("mv: '%s' exist\n", argv[2]);
		close(src);
		close(dst);
		return -1;
	}

	dst = open(argv[2], O_CREATE);
	if(!dst) {
		printf("mv: create '%s' failed\n", argv[2]);
		close(src);
		return -1;
	}

	char *mem = malloc(stat.size);
	if(!mem) {
		printf("mv: malloc failed\n");
		close(src);
		close(dst);
		return -1;
	}
	read(src, mem, stat.size);
	write(dst, mem, stat.size);
	free(mem);
	close(src);
	close(dst);

	unlink(argv[1]);

	return 0;
}

