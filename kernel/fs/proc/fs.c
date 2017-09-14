#include "kernel.h"

extern struct inode_operation proc_sysinfo_inode_op;
extern struct inode_operation proc_proc_inode_op;

void proc_read_inode(struct super_block *sb, struct inode *ip)
{
	ip->mtime = get_systime();
	ip->size = 0;
	if(ip->inum == 1) {
		ip->type = T_FILE;
		ip->i_op = &proc_sysinfo_inode_op;
	}
	else if (ip->inum >= (1 << 16)){
		ip->i_op = &proc_proc_inode_op;
		int low = ip->inum & 0xffff;
		if(low == 1) {
			ip->type = T_DIR;
		}
		else {
			ip->type = T_FILE;
		}
	}
}

void proc_write_inode(struct super_block *sb, struct inode *ip)
{
}

struct inode *proc_ialloc(struct super_block *sb, int type)
{
	return NULL;
}

void proc_read_sb(struct super_block *sb); 
struct super_block_operation proc_sb_op = {
	proc_read_sb,
	proc_ialloc,
	proc_read_inode,
	proc_write_inode,
};

void register_file_system_proc(struct file_system *fs)
{
	fs->reg = 1;
	strcpy(fs->name, "proc");
	fs->s_op = &proc_sb_op;
}

extern struct file_operation proc_file_op;
extern struct inode_operation proc_root_inode_op;

struct inode_operation proc_inode_op = {};

void proc_read_sb(struct super_block *sb) 
{
	sb->s_op = &proc_sb_op;
	sb->i_op = &proc_inode_op;
	sb->f_op = &proc_file_op;
	sb->root->i_op = &proc_root_inode_op;
}

