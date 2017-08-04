#include "kernel.h"

static struct mem_header *free_list = NULL;

void mem_init(char *vstart, char *vend)
{
	char *s = (char *)PG_ROUNDUP((uint)vstart);
	char *e = (char *)PG_ROUNDUP((uint)vend);
	for(; s < e; s += PG_SIZE) {
		mem_free(s);
	}
}

char *mem_alloc() 
{
	struct mem_header *header = free_list;
	if(free_list)
		free_list = free_list->next;
	return (char *)header;
}

void mem_free(char *p)
{
	struct mem_header *header = (struct mem_header *)p;
	header->next = free_list;
	free_list = header;
}

int size_of_free_page()
{
	int size = 0;
	struct mem_header *header = free_list;
	while(header) {
		header = header->next;
		size++;
	}
	return size;
}

int size_of_free_memory() {
	return size_of_free_page() * PG_SIZE;
}

