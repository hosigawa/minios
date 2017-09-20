#ifndef __MALLOC_H__
#define __MALLOC_H__

#include "libc.h"
#include "type.h"

#define HEAD_SZ sizeof(struct malloc_header)

struct malloc_header {
	int size;
	int used;
	struct malloc_header *prev;
	struct malloc_header *next;
	char data[];
};

void malloc_init();
void *malloc(int sz);
void free(void *data);
int memory_size(bool totol);

#endif

