#include "kernel.h"

extern struct CPU cpu;

int (*syscalls[])(void) = {
	[SYS_fork] = sys_fork,
	[SYS_execv] = sys_execv,
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
	[SYS_sleep] = sys_sleep,
	[SYS_stime] = sys_stime,
	[SYS_signal] = sys_signal,
	[SYS_sigret] = sys_sigret,
	[SYS_kill] = sys_kill,
	[SYS_readdir] = sys_readdir,
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
	if(seq == SYS_sigret)
		syscalls[seq]();
	else
		cpu.cur_proc->tf->eax = syscalls[seq]();
}

int sys_fork() 
{
	return fork();
}

int sys_execv()
{
	char *path = (char *)get_arg_uint(0);
	char **argv = (char **)get_arg_uint(1);
	char **envp = (char **)get_arg_uint(2);
	return execv(path, argv, envp);
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
	return f->f_op->read(f, dst, len);
}

int sys_write()
{
	int fd = get_arg_int(0);
	char *src = (char *)get_arg_uint(1);
	int len = get_arg_int(2);

	struct file *f = get_file(fd);
	if(!f)
		return -1;
	return f->f_op->write(f, src, len);
}

int sys_fstat()
{
	int fd = get_arg_int(0);
	struct file_stat *fs = (struct file_stat *)get_arg_uint(1);
	struct file *f = get_file(fd);
	if(!f)
		return -1;
	fs->type = f->ip->type;
	fs->nlink = f->ip->nlink;
	fs->size = f->ip->size;
	fs->ctime = f->ip->ctime;
	fs->mtime = f->ip->mtime;
	fs->atime = f->ip->atime;
	return 0;
}

int sys_pwd()
{
	char *wd = (char *)get_arg_uint(0);
	int off;
	struct inode *dp = cpu.cur_proc->cwd->i_op->dirlookup(cpu.cur_proc->cwd, "..", &off);
	if(!dp)
		return -1;
	if(dp->inum == cpu.cur_proc->cwd->inum) {
		memmove(wd, "/", 2);
		iput(dp);
		return 0;
	}
	struct dirent *de = kalloc();
	if(!de)
		return -1;
	memset(de, 0, PG_SIZE);
	int ret = dp->i_op->readdir(dp, de);
	int i = 0;
	for(i = 0; i < ret; i++) {
		if(de[i].inum == cpu.cur_proc->cwd->inum && strcmp(de[i].name, ".") < 0 && strcmp(de[i].name, "..") < 0) {
			memmove(wd, de[i].name, DIR_NM_SZ);
			iput(dp);
			kfree(de);
			return 0;
		}
	}
	iput(dp);
	kfree(de);
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
	if(dp->type != T_DIR) {
		iput(dp);
		return -2;
	}
	iput(cpu.cur_proc->cwd);
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

int sys_sleep()
{
	int slpms = get_arg_uint(0);
	int tick = slpms;
	while(tick) {
		sleep();
		if(cpu.cur_proc->signal)
			return 0;
		tick--;
	}
	return 0;
}

int sys_stime()
{
	uint *time = (uint *)get_arg_uint(0);
	*time = get_systime();
	return 0;
}

int sys_signal()
{
	uint signal = get_arg_uint(0);
	sig_handler handler = (sig_handler)get_arg_uint(1);

	if(signal == SIG_KILL || signal == SIG_STOP)
		return -1;
	
	sig_handler old = cpu.cur_proc->sig_handlers[signal - 1].handler;
	cpu.cur_proc->sig_handlers[signal - 1].handler = handler;

	return (int)old;
}

int sys_sigret()
{
	struct trap_frame *tf = cpu.cur_proc->tf;
	uint *old_esp = (uint *)tf->esp;
	tf->edx = *(old_esp);
	tf->ecx = *(old_esp + 1);
	tf->eax = *(old_esp + 2);
	tf->eflags = *(old_esp + 3);
	tf->eip = *(old_esp + 4);
	tf->esp += 28;
	return 0;
}

int sys_kill()
{
	int pid = get_arg_int(0);
	uint signal = get_arg_uint(1);
	return kill(pid, signal);
}

int sys_readdir()
{
	int fd = get_arg_int(0);
	struct dirent *de = (struct dirent *)get_arg_uint(1);

	struct file *f = get_file(fd);
	if(!f)
		return -1;
	if(f->ip->type != T_DIR)
		return -1;
	return f->ip->i_op->readdir(f->ip, de);
}

