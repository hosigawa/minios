#ifndef __PROCFS_H__
#define __PROCFS_H__

void init_procfs();
int proc_read(struct inode *ip, char *dst, int len);
int proc_write(struct inode *ip, char *dst, int len);

#endif

