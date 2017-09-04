#ifndef __TYPE_H__
#define __TYPE_H__

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef char bool;

typedef uint pde_t;
typedef uint pte_t;

#define true 1
#define TRUE true

#define false 0
#define FALSE false

#ifndef NULL
	#define NULL 0
	#define null NULL
#endif

struct list_t {
	struct list_t *prev;
	struct list_t *next;
};

#endif

