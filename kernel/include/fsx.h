#ifndef __FSX_H__
#define __FSX_H__

#define NDIRECT 12
#define IPER (BLOCK_SIZE / sizeof(struct minios_inode))
#define IBLOCK(inum,sb) ((inum) / IPER + (sb)->minios_s.inodestart)
#define BPER (BLOCK_SIZE * 8)
#define BBLOCK(bnum,sb) ((bnum) / BPER + (sb)->minios_s.bmapstart)

struct minios_inode {
	short type;           // File type
  	short major;          // Major device number (T_DEV only)
  	short minor;          // Minor device number (T_DEV only)
  	short nlink;          // Number of links to inode in file system
  	uint size;            // Size of file (bytes)
	uint ctime;
	uint mtime;
	uint atime;
  	uint addrs[NDIRECT+14];   // Data block addresses
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

#endif

