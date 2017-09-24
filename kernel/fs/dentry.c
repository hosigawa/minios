#include "kernel.h"

static struct dentry dentry_cache[DENTRY_NUM];

void dentry_init()
{
	int i;
	for(i = 0; i < DENTRY_NUM; i++)
		dentry_cache[i].ref = 0;
}

struct dentry *dalloc(struct dentry *parent, char *name)
{
	int i;
	for(i = 0; i < DENTRY_NUM; i++) {
		if(dentry_cache[i].ref == 0) {
			dentry_cache[i].ref = 1;
			dentry_cache[i].ip = NULL;
			dentry_cache[i].sb = NULL;
			dentry_cache[i].parent = NULL;
			if(parent) {
				dentry_cache[i].parent = parent;
				dentry_cache[i].sb = parent->sb;
				parent->ref++;
				list_add(&parent->subdirs, &dentry_cache[i].child);
			}
			dentry_cache[i].subdirs.next = NULL;
			memset(dentry_cache[i].name, 0, DIR_NM_SZ);
			strcpy(dentry_cache[i].name, name);
			return &dentry_cache[i];
		}
	}
	panic("dentry not enough\n");
}

void dput(struct dentry *de)
{
	if(!de)
		panic("dput null\n");
repeat:
	if(de->ref <= 0)
		panic("dput error\n");
	if((--de->ref) == 0) {
		if(!list_empty(&de->subdirs))
			panic("try to relese a dentry with child:%s\n", de->name);
		if(de->ip){
			iput(de->ip);
		}
		de->ip = NULL;
		de->sb = NULL;
		if(de->parent){
			list_del(&de->child);
			de = de->parent;
			goto repeat;
		}
	}
}

struct dentry *ddup(struct dentry *de)
{
	if(!de || de->ref <= 0)
		panic("ddup error\n");
	de->ref++;
	return de;
}

void dadd(struct dentry *de, struct inode *ip)
{
	//ip->de = de;
	de->ip = ip;
	de->sb = ip->sb;
}

struct dentry *dget(struct dentry *parent, char *name, struct super_block *sb, int inum)
{
	struct dentry *de = dalloc(parent, name);
	struct inode *ip = iget(sb, inum);
	dadd(de, ip);
	return de;
}

