#include "kernel.h"

#define STATIC_PROC_ENTRY 3

static bool name_is_digital(char *name)
{
	while(*name) {
		if(!isdigit(*name))
			return false;
		name++;
	}
	return true;
}

int proc_file_read(struct file *f, char *dst, int len)
{
	int ret = f->ip->i_op->readi(f->ip, dst, f->off, len);
	f->off += ret;

	return ret;
}

int proc_file_write(struct file *f, char *src, int len)
{
	int ret = f->ip->i_op->writei(f->ip, src, f->off, len);
	f->off += ret;

	return ret;
}

int proc_root_readdir(struct file *f, struct dirent *de)
{
	extern struct proc proc_table[MAX_PROC];
	int nr;
	if(f->off == 0) {
		sprintf(de->name, ".");
		f->off++;
		return f->off;
	}
	else if(f->off == 1){
		sprintf(de->name, "..");
		f->off++;
		return f->off;
	}
	else if(f->off < MAX_PROC + 2){
		nr = f->off - 2;
		while(nr < MAX_PROC) {
			f->off++;
			if(proc_table[nr].stat != UNUSED) {
				de->inum = (proc_table[nr].pid << 16) | 1;
				sprintf(de->name, "%d", proc_table[nr].pid);
				return f->off;
			}
			nr = f->off - 2;
		}
	}

	if(f->off == MAX_PROC + 2) {
		sprintf(de->name, "sysinfo");
		f->off++;
		return f->off;
	}

	return -1;
}

struct inode *proc_root_dirlookup(struct inode *dp, char *name, int *off)
{
	struct inode *ip;
	if(!strcmp(name, ".")) {
		ip = idup(dp);
	}
	else if(!strcmp(name, "..")) {
		ip = NULL;
	}
	else if(!strcmp(name, "sysinfo")) {
		ip = iget(dp->sb, 1);
	}
	else if(name_is_digital(name)){
		int pid = atoi(name);
		struct proc *p = get_proc(pid);
		if(!p)
			ip = NULL;
		else
			ip = iget(dp->sb, pid << 16 | 1);
	}
	else 
		ip = NULL;
	return ip;
}

int proc_sysinfo_readi(struct inode *ip, char *dst, int offset, int num)
{
	int wd = 0;
	extern uint _version;
	extern uint user_ticks, kern_ticks, ticks;
	wd += sprintf(dst, "%d %d %d %d %d %d\n", 
					_version,
					PHYSICAL_END, 
					size_of_free_memory(),
					user_ticks,
					kern_ticks,
					ticks
				);
	return wd;
}

struct file_operation proc_sysinfo_file_op = {
	.read = proc_file_read,
	.write = proc_file_write,
};

struct inode_operation proc_sysinfo_inode_op = {
	.f_op = &proc_sysinfo_file_op,
	.readi = proc_sysinfo_readi,
};

struct file_operation proc_root_file_op = {
	proc_file_read,
	proc_file_write,
	proc_root_readdir,
};

struct inode_operation proc_root_inode_op = {
	.f_op = &proc_root_file_op,
	.dirlookup = proc_root_dirlookup,
};

