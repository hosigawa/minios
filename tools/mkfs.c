#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define stat xv6_stat  // avoid clash with host struct stat
#include "fs.h"
#include "type.h"

#ifndef static_assert
#define static_assert(a, b) do { switch (0) case 0: case (a): ; } while (0)
#endif

#define NINODES 200
#define FS_SIZE 1000
#define LOG_SIZE 30

#define min(a, b) (((a) < (b)) ? (a) : (b))

// Disk layout:
// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]

int nbitmap = FS_SIZE/(BLOCK_SIZE*8) + 1;
int ninodeblocks = NINODES / IPER + 1;
int nlog = LOG_SIZE;
int nmeta;    // Number of meta blocks (boot, sb, nlog, inode, bitmap)
int nblocks;  // Number of data blocks

int fsfd;
struct super_block sb;
char zeroes[BLOCK_SIZE];
uint freeinode = 1;
uint freeblock;


void balloc(int);
void wsect(uint, void*);
void winode(uint, struct dinode*);
void rinode(uint inum, struct dinode *ip);
void rsect(uint sec, void *buf);
uint _ialloc(ushort type);
void iappend(uint inum, void *p, int n);
uint create_dir(char *name, uint pinode);

// convert to intel byte order
ushort
xshort(ushort x)
{
  ushort y;
  uchar *a = (uchar*)&y;
  a[0] = x;
  a[1] = x >> 8;
  return y;
}

uint
xint(uint x)
{
  uint y;
  uchar *a = (uchar*)&y;
  a[0] = x;
  a[1] = x >> 8;
  a[2] = x >> 16;
  a[3] = x >> 24;
  return y;
}

int
main(int argc, char *argv[])
{
  int i, cc, fd;
  uint rootino, bin, home, dev, inum, off;
  struct dirent de;
  char buf[BLOCK_SIZE];
  struct dinode din;


  static_assert(sizeof(int) == 4, "Integers must be 4 bytes!");

  if(argc < 2){
    fprintf(stderr, "Usage: mkfs fs.img files...\n");
    exit(1);
  }

  assert((BLOCK_SIZE % sizeof(struct dinode)) == 0);
  assert((BLOCK_SIZE % sizeof(struct dirent)) == 0);

  fsfd = open(argv[1], O_RDWR|O_CREAT|O_TRUNC, 0666);
  if(fsfd < 0){
    perror(argv[1]);
    exit(1);
  }

  // 1 fs block = 1 disk sector
  nmeta = 2 + nlog + ninodeblocks + nbitmap;
  nblocks = FS_SIZE - nmeta;

  sb.size = xint(FS_SIZE);
  sb.nblocks = xint(nblocks);
  sb.ninodes = xint(NINODES);
  sb.nlog = xint(nlog);
  sb.logstart = xint(2);
  sb.inodestart = xint(2+nlog);
  sb.bmapstart = xint(2+nlog+ninodeblocks);

  printf("nmeta %d (boot, super, log blocks %u inode blocks %u, bitmap blocks %u) blocks %d total %d\n",
         nmeta, nlog, ninodeblocks, nbitmap, nblocks, FS_SIZE);

  freeblock = nmeta;     // the first free block that we can allocate

  for(i = 0; i < FS_SIZE; i++)
    wsect(i, zeroes);

  memset(buf, 0, sizeof(buf));
  memmove(buf, &sb, sizeof(sb));
  wsect(1, buf);

  rootino = _ialloc(T_DIR);
  assert(rootino == ROOTINO);

  bzero(&de, sizeof(de));
  de.inum = xshort(rootino);
  strcpy(de.name, ".");
  iappend(rootino, &de, sizeof(de));

  bzero(&de, sizeof(de));
  de.inum = xshort(rootino);
  strcpy(de.name, "..");
  iappend(rootino, &de, sizeof(de));

  bin = create_dir("bin", rootino);
  home = create_dir("home", rootino);
  dev = create_dir("dev", rootino);

  uint target = 0;
  for(i = 2; i < argc; i++){
    assert(index(argv[i], '/') == 0);

    if((fd = open(argv[i], 0)) < 0){
      perror(argv[i]);
      exit(1);
    }

    // Skip leading _ in name when writing to file system.
    // The binaries are named _rm, _cat, etc. to keep the
    // build operating system from trying to execute them
    // in place of system binaries like rm and cat.
    if(argv[i][0] == '_') {
      ++argv[i];
	  target = bin;
	}
    else {
	  target = home;
	}

    inum = _ialloc(T_FILE);

    bzero(&de, sizeof(de));
    de.inum = xshort(inum);
    strncpy(de.name, argv[i], DIR_NM_SZ);
    iappend(target, &de, sizeof(de));

    while((cc = read(fd, buf, sizeof(buf))) > 0)
      iappend(inum, buf, cc);

    close(fd);
  }

  // fix size of root inode dir
  rinode(rootino, &din);
  off = xint(din.size);
  off = ((off/BLOCK_SIZE) + 1) * BLOCK_SIZE;
  din.size = xint(off);
  winode(rootino, &din);

  balloc(freeblock);

  exit(0);
}

void
wsect(uint sec, void *buf)
{
  if(lseek(fsfd, sec * BLOCK_SIZE, 0) != sec * BLOCK_SIZE){
    perror("lseek");
    exit(1);
  }
  if(write(fsfd, buf, BLOCK_SIZE) != BLOCK_SIZE){
    perror("write");
    exit(1);
  }
}

uint create_dir(char *name, uint pinode)
{
  struct dirent de;
  uint new_dir = _ialloc(T_DIR);

  bzero(&de, sizeof(de));
  de.inum = xshort(new_dir);
  strcpy(de.name, ".");
  iappend(new_dir, &de, sizeof(de));

  bzero(&de, sizeof(de));
  de.inum = xshort(pinode);
  strcpy(de.name, "..");
  iappend(new_dir, &de, sizeof(de));
  
  bzero(&de, sizeof(de));
  de.inum = xshort(new_dir);
  strcpy(de.name, name);
  iappend(pinode, &de, sizeof(de));

  return new_dir;
}

void
winode(uint inum, struct dinode *ip)
{
  char buf[BLOCK_SIZE];
  uint bn;
  struct dinode *dip;

  bn = IBLOCK(inum);
  rsect(bn, buf);
  dip = ((struct dinode*)buf) + (inum % IPER);
  *dip = *ip;
  wsect(bn, buf);
}

void
rinode(uint inum, struct dinode *ip)
{
  char buf[BLOCK_SIZE];
  uint bn;
  struct dinode *dip;

  bn = IBLOCK(inum);
  rsect(bn, buf);
  dip = ((struct dinode*)buf) + (inum % IPER);
  *ip = *dip;
}

void
rsect(uint sec, void *buf)
{
  if(lseek(fsfd, sec * BLOCK_SIZE, 0) != sec * BLOCK_SIZE){
    perror("lseek");
    exit(1);
  }
  if(read(fsfd, buf, BLOCK_SIZE) != BLOCK_SIZE){
    perror("read");
    exit(1);
  }
}

uint
_ialloc(ushort type)
{
  uint inum = freeinode++;
  struct dinode din;

  bzero(&din, sizeof(din));
  din.type = xshort(type);
  din.nlink = xshort(1);
  din.size = xint(0);
  winode(inum, &din);
  return inum;
}

void
balloc(int used)
{
  uchar buf[BLOCK_SIZE];
  int i;

  printf("balloc: first %d blocks have been allocated\n", used);
  assert(used < BLOCK_SIZE*8);
  bzero(buf, BLOCK_SIZE);
  for(i = 0; i < used; i++){
    buf[i/8] = buf[i/8] | (0x1 << (i%8));
  }
  printf("balloc: write bitmap block at sector %d\n", sb.bmapstart);
  wsect(sb.bmapstart, buf);
}

void
iappend(uint inum, void *xp, int n)
{
  char *p = (char*)xp;
  uint fbn, off, n1;
  struct dinode din;
  char buf[BLOCK_SIZE];
  uint indirect[NINDIRECT];
  uint x;

  rinode(inum, &din);
  off = xint(din.size);
  // printf("append inum %d at off %d sz %d\n", inum, off, n);
  while(n > 0){
    fbn = off / BLOCK_SIZE;
    assert(fbn < MAXFILE);
    if(fbn < NDIRECT){
      if(xint(din.addrs[fbn]) == 0){
        din.addrs[fbn] = xint(freeblock++);
      }
      x = xint(din.addrs[fbn]);
    } else {
      if(xint(din.addrs[NDIRECT]) == 0){
        din.addrs[NDIRECT] = xint(freeblock++);
      }
      rsect(xint(din.addrs[NDIRECT]), (char*)indirect);
      if(indirect[fbn - NDIRECT] == 0){
        indirect[fbn - NDIRECT] = xint(freeblock++);
        wsect(xint(din.addrs[NDIRECT]), (char*)indirect);
      }
      x = xint(indirect[fbn-NDIRECT]);
    }
    n1 = min(n, (fbn + 1) * BLOCK_SIZE - off);
    rsect(x, buf);
    bcopy(p, buf + off - (fbn * BLOCK_SIZE), n1);
    wsect(x, buf);
    n -= n1;
    off += n1;
    p += n1;
  }
  din.size = xint(off);
  winode(inum, &din);
}

