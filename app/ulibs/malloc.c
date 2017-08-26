#include "malloc.h"
#include "usyscall.h"
#include "mmu.h"
#include "stdio.h"

static struct malloc_header *head = NULL;

static void check_head()
{
	int ret = 0;
	struct malloc_header *tmp;
	if(!head || head->used == 1)
		return;
	if((head->size + HEAD_SZ) % PG_SIZE == 0) {
		tmp = head;
		head = head->prev;
		if(head)
			head->next = NULL;
		ret = sbrk(-(tmp->size + HEAD_SZ));
	}
	else {
		ret = sbrk(-(int)PG_ROUNDDOWN(head->size));
		head->size -= (int)PG_ROUNDDOWN(head->size);
	}
	if(ret < 0) {
		printf("check sbrk error\n");
		exit();
	}
}

void *malloc(int sz)
{
	sz += HEAD_SZ;
	struct malloc_header *p = head;
	while(p) {
		if(p->used == 0 && p->size >= sz)
			break;
		p = p->prev;
	}

	if(!p) {
		p = (struct malloc_header *)sbrk((int)PG_ROUNDUP(sz));
		if((int)p < 0) {
			printf("malloc sbrk error\n");
			return NULL;
		}
		if(head && head->used == 0) {
			head->size += PG_ROUNDUP(sz);
			head->used = 1;
			return head->data;
		}
		p->size = PG_ROUNDUP(sz) - HEAD_SZ;
		p->used = 1;
		p->next = NULL;
		p->prev = head;
		if(head)
			head->next = p;
		head = p;
		return head->data;
	}

	struct malloc_header *np;
	np = (struct malloc_header *)(p->data + sz - HEAD_SZ);
	np->size = p->size - sz;
	np->used = 0;
	np->prev = p;
	np->next = p->next;
	if(np->next)
		np->next->prev = np;
	p->size = sz - HEAD_SZ;
	p->used = 1;
	p->next = np;
	if(p == head)
		head = np;
	return p->data;
}

void free(void *data)
{
	if(data == 0)
		return;
	bool exist = false;
	struct malloc_header *p = (struct malloc_header *)(data - HEAD_SZ);
	struct malloc_header *tmp = head;
	while(tmp) {
		if(tmp == p) {
			exist = true;
			break;
		}
		tmp = tmp->prev;
	}
	if(!exist) {
		printf("free not exist: %p\n", data);
		exit();
	}
	if(p->used == 0){
		printf("double free: %p\n", data);
		exit();
	}
	p->used = 0;
	if(p->next && p->next->used == 0) {
		p->size += p->next->size + HEAD_SZ;
		if(p->next == head)
			head = p;
		p->next = p->next->next;
		if(p->next)
			p->next->prev = p;
	}
	if(p->prev && p->prev->used == 0) {
		p->prev->size += p->size + HEAD_SZ;
		if(p == head)
			head = p->prev;
		p->prev->next = p->next;
		if(p->next)
			p->next->prev = p->prev;
	}
	check_head();
}

int memory_size(bool totol)
{
	int size = 0;
	struct malloc_header *p = head;
	if(!p)
		return 0;
	while(p) {
		if(totol || p->used == 0)
			size += p->size;
		p = p->prev;
	}
	return size;
}

