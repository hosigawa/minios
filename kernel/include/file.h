#ifndef __FILE_H__
#define __FILE_H__

#include "fs.h"

#define MAX_DEV 100
#define NFILE 1024

#define CONSOLE 1
#define PROCINFO 2
#define SYSINFO 3

#define FD_NONE 0
#define FD_PIPE 1
#define FD_INODE 2

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

struct file;
struct file_operation {
	int (*read)(struct file *f, char *dst, int len);
	int (*write)(struct file *f, char *src, int len);
};

struct file {
	int type;
	struct inode *ip;
	struct file_operation *f_op;
	int ref;
	int off;
};


int fd_alloc(struct file *f);
struct file *file_alloc();
struct file *file_dup(struct file *f);
struct inode *file_create(char *path, int type, int major, int minor);
int file_open(char *path, int mode);
int file_close(struct file *f);
int file_mknod(char *path, int major, int minor);
int file_mkdir(char *path, int major, int minor);
int file_unlink(char *path);
struct file *get_file(int fd);

#endif

