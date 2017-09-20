#include "kernel.h"

__attribute__((__aligned__(PG_SIZE)))
static char mem_map[PHYSICAL_END >> 12] = {0};
static uint k_alloc = 0;
static uint v_alloc = 0;
extern char end[];
extern char stack[];

void mem_init()
{
	if(PHYSICAL_END % PG_SIZE)
		panic("physical mem not align\n");
	uint i;
	k_alloc = V2P(PG_ROUNDUP((int)end));
	for(i = 0; i < k_alloc; i += PG_SIZE) {
		mem_map[i >> 12] = 100;
	}
	for(i = k_alloc; i < (uint)PHYSICAL_END; i += PG_SIZE) {
		mem_map[i >> 12] = 0;
	}
	v_alloc = min(PHYSICAL_END, LOW_MEM);
	//printf("mem_init: free page is %d\n", size_of_free_page());
}

void *kalloc() 
{
	uint i;
	for(i = k_alloc; i < v_alloc; i += PG_SIZE) {
		if(!mem_map[i >> 12]) {
			mem_map[i >> 12]++;
			return P2V(i);
		}
	}
	return NULL;
}

void kfree(void *p)
{
	if((uint)p % PG_SIZE || p < P2V(k_alloc) || p >= P2V(v_alloc))
    	panic("kfree error: %p\n", p);
	if(!mem_map[V2P(p) >> 12])
    	panic("kfree free page: %p\n", p);
	mem_map[V2P(p) >> 12] = 0;
}

uint valloc()
{
	uint i;
	for(i = PHYSICAL_END - PG_SIZE; i > k_alloc; i -= PG_SIZE) {
		if(!mem_map[i >> 12]) {
			mem_map[i >> 12] = 1;
			return i;
		}
	}
	return NULL;
}

uint vdup(uint p)
{
	if(p % PG_SIZE || p < k_alloc)
    	panic("vdup error: %p\n", p);
	if(mem_map[p >> 12] < 1)
		panic("vdup :%p\n", p);
	mem_map[p >> 12]++;
	return p;
}

void vfree(uint p)
{
	if(p % PG_SIZE || p < k_alloc)
    	panic("vfree error: %p\n", p);
	if(!mem_map[p >> 12])
    	panic("vfree free page: %p\n", p);
	if(mem_map[p >> 12] == 1){
		mem_zero(p);
	}
	mem_map[p >> 12]--;
}

int size_of_free_page()
{
	uint i;
	int size = 0;
	for(i = k_alloc; i < PHYSICAL_END; i += PG_SIZE) {
		if(!mem_map[i >> 12])
			size++;
	}
	return size;
}

int size_of_free_memory() {
	return size_of_free_page() * PG_SIZE;
}

void mem_zero(uint p)
{
	kmap_atomic(USER0, p);
	memset(USER0, 0, PG_SIZE);
	unkmap_atomic(USER0);
}

