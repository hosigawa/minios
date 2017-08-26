#ifndef __PROCFS_H__
#define __PROCFS_H__

void init_procfs();
int procinfo_read(struct inode *ip, char *dst, int off, int len);
int procinfo_write(struct inode *ip, char *dst, int off, int len);
int sysinfo_read(struct inode *ip, char *dst, int off, int len);
int sysinfo_write(struct inode *ip, char *dst, int off, int len);

#endif

