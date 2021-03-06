#ifndef __FS_H__
#define __FS_H__

#include "type.h"
#include "list.h"
#include "../fs/minios/minios.h"

#define BLOCK_BUF_NUM 30
#define BLOCK_SIZE 1024
#define B_VALID 0x2  // buffer has been read from disk
#define B_DIRTY 0x4  // buffer needs to be written to disk
#define INODE_NUM 512
#define DENTRY_NUM 512
#define SUPER_BLOCK_NUM 64
#define FILE_SYSTEM_NUM 64
#define I_VALID 0x2

#define T_DIR 1
#define T_FILE 2
#define T_DEV 3
#define T_LINK 4

#define DIR_NM_SZ 28
#define ROOT_DEV 0
#define ROOT_INO 1

#define CHECK_OP(op) if(!(op)){panic("lost operation\n");}

#define MAX_DEV 100
#define NFILE 1024

#define MAJOR(x) ((x) >> 16)
#define MINOR(x) ((x) | 0xffff)

#define DEV_NULL 100
#define DEV_CONSOLE 101

#define FD_NONE 0
#define FD_PIPE 1
#define FD_INODE 2

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

struct block_buf {
	int flags;
	int dev;
	int sec;
	int ref;
	struct block_buf *prev;
	struct block_buf *next;
	struct block_buf *qnext;
	char data[BLOCK_SIZE];
};

struct dirent {
	int inum;
	char name[DIR_NM_SZ];
};

struct file_system {
	int reg;
	char name[64];
	struct super_block_operation *s_op;
};

struct inode;
struct super_block;
struct dentry {
	int ref;
	struct inode *ip;
	struct super_block *sb;
	struct dentry *parent;
	struct list_t child;
	struct list_t subdirs;
	char name[DIR_NM_SZ];
};

struct file_operation;
struct inode_operation {
	struct file_operation *f_op;
	struct dentry *(*lookup)(struct inode *dp, struct dentry *de, char *name, int *off);
	void (*itrunc)(struct inode *ip);
	void (*dirlink)(struct inode *dp, char *name, int inum);
	struct dentry *(*create)(struct inode *dp, struct dentry *de, char *name, int type);
	int (*unlink)(struct inode *dp, struct dentry *de, char *name);
	int (*readi)(struct inode *ip, char *dst, int offset, int num);
	int (*writei)(struct inode *ip, char *src, int offset, int num);
};

struct inode {
	uint dev;           // Device number
	uint inum;          // Inode number
	int ref;            // Reference count
	int flags;          // I_VALID
	short type;           // File type
  	short nlink;          // Number of links to inode in file system
  	uint size;            // Size of file (bytes)
	uint ctime;
	uint mtime;
	uint atime;
	//struct dentry *de;
	struct super_block *sb;
	struct inode_operation *i_op;

	union {
		struct minios_inode_info minios_i;
	};
};

struct file;
struct file_operation {
	int (*read)(struct file *f, char *dst, int len);
	int (*write)(struct file *f, char *src, int len);
	int (*readdir)(struct file *f, struct dirent *de);
};

struct file {
	int type;
	struct dentry *de;
	struct file_operation *f_op;
	int ref;
	int off;
};

struct super_block;
struct super_block_operation {
	void (*read_sb)(struct super_block *sb);
	struct inode *(*ialloc)(struct super_block *sb, int type);
	void (*read_inode)(struct super_block *sb, struct inode *ip);
	void (*write_inode)(struct super_block *sb, struct inode *ip);
};

struct super_block {
	int dev;
	struct dentry *mount;
	struct super_block_operation *s_op;
	struct inode_operation *i_op;

	union {
		struct minios_super_block minios_s;
	};
};

struct proc_struct {
	int base;
	char *name;
};

struct block_buf *bread(int dev, int sec);
void bwrite(struct block_buf *buf);
struct block_buf *bget(int dev, int sec);
void brelse(struct block_buf *buf);

void init_fs();
void mount_root();
int mount_fs(char *path, char *fs_name);
struct inode *iget(struct super_block *sb, int inum);
struct inode *idup(struct inode *ip);
void iput(struct inode *ip);

void dentry_init();
struct dentry *dalloc(struct dentry *parent, char *name);
struct dentry *ddup(struct dentry *de);
void dput(struct dentry *de);
void dadd(struct dentry *de, struct inode *ip);
struct dentry *dget(struct dentry *parent, char *name, struct super_block *sb, int inum);

void d_path(struct dentry *de, char *path);
struct dentry *namex(char *path, char *name, bool bparent);
struct dentry *namei(char *path);
struct dentry *namep(char *path, char *name);

int fd_alloc(struct file *f);
struct file *file_alloc();
struct file *file_dup(struct file *f);
struct dentry *file_create(char *path, int type);
int file_open(char *path, int mode);
int file_close(struct file *f);
int file_mknod(char *path, int major, int minor);
int file_mkdir(char *path);
int file_unlink(char *path);
struct file *get_file(int fd);

struct proc_struct *match_proc_struct(struct proc_struct *ps, int len, char *name);
struct proc_struct *get_proc_struct(struct proc_struct *ps, int len, int seq);

void init_dev();
void register_devop(int dev, struct inode_operation *i_op);
struct inode_operation *get_devop(int dev);

#endif

