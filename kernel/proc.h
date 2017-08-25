#ifndef __PROC_H__
#define __PROC_H__

#include "file.h"

#define MAX_PROC 64
#define PROC_NM_SZ 64
#define NOFILE 16

enum PROC_STATUS {
	UNUSED = 0,
	EMBRYO,
	READY,
	RUNNING,
	SLEEPING,
	ZOMBIE,
};

struct context {
	uint edi;
	uint esi;
	uint ebx;
	uint ebp;
	uint eip;
};

struct trap_frame;

struct proc {
	int pid;
	char name[PROC_NM_SZ];
	int killed;
	uint kstack;
	uint vend;
	struct trap_frame *tf;
	struct context *context;
	int stat;
	pde_t *pgdir;
	struct file *ofile[NOFILE];
	struct inode *cwd;
	struct proc *parent;
	void *sleep_chan;
};

int fork();
void forkret();
struct proc *alloc_proc();
void scheduler() __attribute__((noreturn));
void sched();
void yield();
void user_init();
void sleep(void *chan);
void wakeup(void *chan);
int exec(char *path, char **argv);
void exit();
int wait();


int proc_read(struct inode *ip, char *dst, int len);
int proc_write(struct inode *ip, char *dst, int len);

#endif

