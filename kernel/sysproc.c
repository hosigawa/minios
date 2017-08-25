#include "kernel.h"

extern struct CPU cpu;

int (*syscalls[])(void) = {
	[SYS_fork] = sys_fork,
	[SYS_exec] = sys_exec,
	[SYS_exit] = sys_exit,
	[SYS_wait] = sys_wait,
	[SYS_open] = sys_open,
	[SYS_close] = sys_close,
	[SYS_dup] = sys_dup,
	[SYS_mknod] = sys_mknod,
	[SYS_read] = sys_read,
	[SYS_write] = sys_write,
	[SYS_fstat] = sys_fstat,
	[SYS_pwd] = sys_pwd,
	[SYS_mkdir] = sys_mkdir,
	[SYS_chdir] = sys_chdir,
	[SYS_unlink] = sys_unlink,
	[SYS_sbrk] = sys_sbrk,
	[SYS_ps] = sys_ps,
};

int get_arg_int(int n)
{
	//todo check user mem boundray
	return *(int *)(cpu.cur_proc->tf->esp + 4 + 4*n);
}

uint get_arg_uint(int n)
{
	//todo check user mem boundray
	return *(uint *)(cpu.cur_proc->tf->esp + 4 + 4*n);
}

char *get_arg_str(int n)
{
	//todo check user mem boundray
	return *(char **)(cpu.cur_proc->tf->esp + 4 + 4*n);
}

void sys_call()
{
	int seq = cpu.cur_proc->tf->eax;
	if(seq <= 0 || seq >= (sizeof(syscalls)/sizeof(syscalls[0]))) {
		err_info("unknow syscall %d\n", seq);
		cpu.cur_proc->tf->eax = -1;
	}
	cpu.cur_proc->tf->eax = syscalls[seq]();
}

int sys_fork() 
{
	return fork();
}

int sys_exec()
{
	char *path = (char *)get_arg_uint(0);
	char **argv = (char **)get_arg_uint(1);
	return exec(path, argv);
}

int sys_exit()
{
	exit();
	return 0;
}

int sys_wait()
{
	return wait();
}

int sys_ps()
{
	struct proc_info *pi = (struct proc_info *)get_arg_uint(0);
	int size = get_arg_int(1);
	
	extern struct proc proc_table[];
	struct proc *p;
	int i = 0, j = 0; 
	for(; i < MAX_PROC; i++) {
		p = proc_table + i;
		if(p->stat != UNUSED) {
			pi[j].pid = p->pid;
			memmove(pi[j].name, p->name, PROC_NM_SZ);
			pi[j].vsz = p->vend - USER_LINK;
			pi[j].stat = p->stat;
			pi[j].ppid = p->parent ? p->parent->pid : 0;

			j++;
			if(j >= size)
				break;
		}
	}
	return 0;
}

int sys_open()
{
	char *path = (char *)get_arg_uint(0);
	int mode = get_arg_int(1);
	return file_open(path, mode);
}

int sys_close()
{
	int fd = get_arg_int(0);
	struct file *f = get_file(fd);
	if(!f)
		return -1;
	cpu.cur_proc->ofile[fd] = NULL;
	return file_close(f);
}

int sys_dup()
{
	int fd = get_arg_int(0);
	struct file *f = get_file(fd);
	if(!f)
		return -1;
	fd = fd_alloc(f);
	file_dup(f);
	return fd;
}

int sys_mknod()
{
	char *path = (char *)get_arg_uint(0);
	int major = get_arg_int(1);
	int minor = get_arg_int(2);
	return file_mknod(path, major, minor);
}

int sys_read()
{
	int fd = get_arg_int(0);
	char *dst = (char *)get_arg_uint(1);
	int len = get_arg_int(2);

	struct file *f = get_file(fd);
	if(!f)
		return -1;
	return file_read(f, dst, len);
}

int sys_write()
{
	int fd = get_arg_int(0);
	char *src = (char *)get_arg_uint(1);
	int len = get_arg_int(2);

	struct file *f = get_file(fd);
	if(!f)
		return -1;
	return file_write(f, src, len);
}

int sys_fstat()
{
	int fd = get_arg_int(0);
	struct file_stat *fs = (struct file_stat *)get_arg_uint(1);
	struct file *f = get_file(fd);
	if(!f)
		return -1;
	read_inode(f->ip);
	fs->type = f->ip->de.type;
	fs->nlink = f->ip->de.nlink;
	fs->size = f->ip->de.size;
	return 0;
}

int sys_pwd()
{
	char *wd = (char *)get_arg_uint(0);
	int off;
	struct inode *dp = dir_lookup(cpu.cur_proc->cwd, "..", &off);
	read_inode(dp);
	if(!dp)
		return -1;
	if(dp->inum == cpu.cur_proc->cwd->inum) {
		memmove(wd, "/", 2);
		irelese(dp);
		return 0;
	}
	struct dirent de;
	off = 0;
	while(readi(dp, (char *)&de, off, sizeof(de)) == sizeof(de)) {
		if(de.inum == cpu.cur_proc->cwd->inum && strcmp(de.name, ".") < 0 && strcmp(de.name, "..") < 0) {
			memmove(wd, de.name, strlen(de.name)+1);
			irelese(dp);
			return 0;
		}
		off += sizeof(de);
	}
	irelese(dp);
	return -2;
}

int sys_mkdir()
{
	char *path = (char *)get_arg_uint(0);
	return file_mkdir(path, 0, 0);
}

int sys_chdir()
{
	char *path = (char *)get_arg_uint(0);
	struct inode *dp = namei(path);
	if(!dp)
		return -1;
	read_inode(dp);
	if(dp->de.type != T_DIR) {
		irelese(dp);
		return -2;
	}
	irelese(cpu.cur_proc->cwd);
	cpu.cur_proc->cwd = dp;
	return 0;
}

int sys_unlink()
{
	char *path = (char *)get_arg_uint(0);
	return file_unlink(path);
}

int sys_sbrk()
{
	int addsz = get_arg_int(0);
	if(abs(addsz) % PG_SIZE)
		return -1;
	int addr = cpu.cur_proc->vend;
	int ret = resize_uvm(cpu.cur_proc->pgdir, addr, addr + addsz);
	if(ret < 0)
		return ret;
	cpu.cur_proc->vend = ret;
	swtch_uvm(cpu.cur_proc);
	return addr;
}

