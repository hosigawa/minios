#ifndef __FS_H__
#define __FS_H__

#define BLOCK_BUF_NUM 30
#define BLOCK_SIZE 512
#define B_VALID 0x2  // buffer has been read from disk
#define B_DIRTY 0x4  // buffer needs to be written to disk
#define NDIRECT 12
#define INODE_NUM 50

struct block_buf {
	int flags;
	int dev;
	int sec;
	struct block_buf *prev;
	struct block_buf *next;
	struct block_buf *qnext;
	char data[BLOCK_SIZE];
	int ref;
};

struct dinode {
	short type;           // File type
  	short major;          // Major device number (T_DEV only)
  	short minor;          // Minor device number (T_DEV only)
  	short nlink;          // Number of links to inode in file system
  	uint size;            // Size of file (bytes)
  	uint addrs[NDIRECT+1];   // Data block addresses
};

struct inode {
  	uint dev;           // Device number
  	uint inum;          // Inode number
  	int ref;            // Reference count
  	int flags;          // I_VALID

	struct dinode di;
};

struct super_block {
	uint size;         // Size of file system image (blocks)
  	uint nblocks;      // Number of data blocks
  	uint ninodes;      // Number of inodes.
  	uint nlog;         // Number of log blocks
  	uint logstart;     // Block number of first log block
  	uint inodestart;   // Block number of first inode block
  	uint bmapstart;    // Block number of first free map block
};

void binit();
struct block_buf *bread(int dev, int sec);
void bwrite(int dev, int sec);
struct block_buf *bget(int dev, int sec);
void brelse(struct block_buf *buf);

void init_fs(int dev);
void readsb(int dev, struct super_block *sb);
struct inode *iget(int dev, int inum);
void irelese(struct inode *ip);
void readi(struct inode *ip, char *dst);
void writei(struct inode *ip, char *dst);
int get_block_num(struct inode *ip, int n);

#endif

