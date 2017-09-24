
#define BLOCK_BUF_NUM 30
#define BLOCK_SIZE 1024
#define B_VALID 0x2  // buffer has been read from disk
#define B_DIRTY 0x4  // buffer needs to be written to disk
#define NDIRECT 12
#define NINDIRECT (BLOCK_SIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)
#define INODE_NUM 128
#define SUPER_BLOCK_NUM 64
#define FILE_SYSTEM_NUM 64
#define I_VALID 0x2

#define T_DIR 1
#define T_FILE 2
#define T_DEV 3
#define DIR_NM_SZ 28
#define ROOT_DEV 1
#define ROOT_INO 1
#define IPER (BLOCK_SIZE / sizeof(struct minios_inode))
#define IBLOCK(inum,sb) ((inum) / IPER + (sb)->minios_s.inodestart)
#define BPER (BLOCK_SIZE * 8)
#define BBLOCK(bnum,sb) ((bnum) / BPER + (sb)->minios_s.bmapstart)

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef char bool;

struct dirent {
	int inum;
	char name[DIR_NM_SZ];
};

struct minios_inode {
	short type;           // File type
	short nlink;          // Number of links to inode in file systeum
	short uid;
	short gid;
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

