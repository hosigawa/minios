#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "mmu.h"

struct mem_header{
	struct mem_header *next;
};

void mem_init();
char *mem_alloc();
void mem_free(void *p);
int size_of_free_memory();
int size_of_free_page();

#endif

