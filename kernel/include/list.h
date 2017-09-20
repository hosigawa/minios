#ifndef __LIST_H__
#define __LIST_H__

#include "type.h"

#define offset_member(type, member) ((int)&((type *)0)->member)

#define list_entry(p, type, member) ({\
		const typeof(((type *)0)->member) *_mp = (p);\
		(type *)((char *)_mp - offset_member(type, member));})
	

#define list_empty(t) (!((t)->next) ? true : false)

struct list_t {
	struct list_t *prev;
	struct list_t *next;
};

static inline void list_add(struct list_t *head, struct list_t *list)
{
	list->next = head->next;
	if(list->next)
		list->next->prev = list;
	list->prev = head;
	head->next = list;
}

static inline void list_del(struct list_t *list)
{
	if(list->next)
		list->next->prev = list->prev;
	if(list->prev)
		list->prev->next = list->next;
	list->prev = NULL;
	list->next = NULL;
}

#endif

