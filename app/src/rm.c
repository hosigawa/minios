#include "stdio.h"

int main(int argc, char *argv[])
{
	int i, ret;
	if(argc < 2) {
		printf("Usage: rm files...\n");
		return -1;
	}

	for(i = 1; i < argc; i++) {
		ret = unlink(argv[i]);
		if(ret == -5) {
			printf("rm: %s: Directory not empty\n", argv[i]);
		}
		else if(ret < 0) {
			printf("rm: %s: remove failed\n", argv[i]);
		}
	}
	return 0;
}

