#ifndef __MINIOSFS_H__
#define __MINIOSFS_H__

#define NDIRECT 12
#define IPER (BLOCK_SIZE / sizeof(struct minios_inode))
#define IBLOCK(inum,sb) ((inum) / IPER + (sb)->minios_s.inodestart)
#define BPER (BLOCK_SIZE * 8)
#define BBLOCK(bnum,sb) ((bnum) / BPER + (sb)->minios_s.bmapstart)

struct inode;
struct super_block;

struct minios_inode {
	short type;           // File type
	short nlink;          // Number of links to inode in file system
	short uid;
	short gid;
  	uint size;            // Size of file (bytes)
	uint ctime;
	uint mtime;
	uint atime;
  	uint addrs[NDIRECT+14];   // Data block addresses
};

struct minios_inode_info {
	uint addrs[NDIRECT + 1];
};

struct minios_super_block {
	uint size;         // Size of file system image (blocks)
  	uint nblocks;      // Number of data blocks
  	uint ninodes;      // Number of inodes.
  	uint nlog;         // Number of log blocks
  	uint logstart;     // Block number of first log block
  	uint inodestart;   // Block number of first inode block
  	uint bmapstart;    // Block number of first free map block
};

void minios_read_inode(struct super_block *sb, struct inode *ip);
void minios_write_inode(struct super_block *sb, struct inode *ip);
int minios_readi(struct inode *ip, char *dst, int offset, int num);
int minios_writei(struct inode *ip, char *src, int offset, int num);
uint minios_bmap(struct super_block *sb, struct inode *ip, int n);
void minios_bfree(struct super_block *sb, uint n);
void minios_dirlink(struct inode *dp, char *name, int inum);
struct dentry *minios_lookup(struct inode *ip, struct dentry *de, char *name, int *off);
struct inode *minios_ialloc(struct super_block *sb, int type);


#endif

