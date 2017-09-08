#ifndef __MEMORY_H__
#define __MEMORY_H__

void mem_init();
void *kalloc();
void kfree(void *p);
uint valloc();
uint vdup(uint p);
void vfree(uint p);
void mem_zero(uint p);
int size_of_free_memory();
int size_of_free_page();

#endif

