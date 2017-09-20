#include "kernel.h"

extern struct CPU cpu;
struct dentry *root_dentry;

struct super_block super_blocks[SUPER_BLOCK_NUM];
struct file_system file_systems[FILE_SYSTEM_NUM];
void init_file();
void binit();
void register_file_system_minios(struct file_system *fs);
void register_file_system_proc(struct file_system *fs);

struct super_block *get_sb(int dev)
{
	int i;
	for(i = 0; i < SUPER_BLOCK_NUM; i++) {
		if(super_blocks[i].dev == dev)
			return &super_blocks[i];
	}
	if(dev == 0)
		panic("super_blocks not enough\n");
	return NULL;
}

struct file_system *get_fs_type(char *fs_name)
{
	int i;
	for(i = 0; i < FILE_SYSTEM_NUM; i++) {
		if(!strcmp(file_systems[i].name, fs_name))
			return &file_systems[i];
	}
	return NULL;
}

struct file_system *get_free_fs()
{
	int i;
	for(i = 0; i < FILE_SYSTEM_NUM; i++) {
		if(file_systems[i].reg == 0)
			return &file_systems[i];
	}
	panic("get_free_fs\n");
}

void register_file_system()
{
	struct file_system *fs;
	fs = get_free_fs();
	register_file_system_minios(fs);
	fs = get_free_fs();
	register_file_system_proc(fs);
}

void init_fs()
{
	int i;
	for(i = 0; i < SUPER_BLOCK_NUM; i++) {
		super_blocks[i].dev = 0;
	}
	for(i = 0; i < FILE_SYSTEM_NUM; i++) {
		file_systems[i].reg = 0;
	}
	binit();
	init_file();
	dentry_init();
	register_file_system();
}

void mount_root()
{
	struct file_system *fs = get_fs_type("minios");
	if(!fs)
		panic("can't find file_system minios\n");
	struct super_block *sb = get_sb(0);
	sb->dev = ROOT_DEV;
	fs->s_op->read_sb(sb);
	sb->mount = dalloc(NULL, "/");
	root_dentry = sb->mount;
	struct inode *ip = iget(sb, ROOT_INO);
	ip->sb = sb;
	ip->i_op = sb->i_op;
	sb->s_op->read_inode(sb, ip);
	dadd(sb->mount, ip);
	cpu.cur_proc->cwd = ddup(sb->mount);
	cpu.cur_proc->exe = ddup(sb->mount);

	printf("mount root file system 'minios' success\n");
}

int mount_fs(char *path, char *fs_name)
{
	static int dev = ROOT_DEV + 1;

	struct dentry *dp = namei(path);
	if(!dp) {
		printf("%s not exists\n", path);
		return -1;
	}
	if(dp->ip->type != T_DIR) {
		dput(dp);
		printf("%s is not direct\n", path);
		return -1;
	}
	struct file_system *fs = get_fs_type(fs_name);
	if(!fs) {
		dput(dp);
		printf("file_system '%s' may not register\n", fs_name);
		return -1;
	}
	struct super_block *sb = get_sb(0);
	sb->dev = dev++;
	sb->mount = ddup(dp);
	fs->s_op->read_sb(sb);
	
	dput(dp);
	
	printf("mount file system '%s' success\n", fs_name);
	return 0;
}

char *path_decode(char *path, char *name)
{
	char *sub_path;
	if(*path == '/')
		path++;
	if(!*path)
		return NULL;
	sub_path = path;
	while(*path != '/' && *path)
		path++;
	int len = path - sub_path;
	if(name) {
		if(len >= DIR_NM_SZ) {
			memmove(name, sub_path, DIR_NM_SZ);
		}
		else {
			memmove(name, sub_path, len);
			name[len] = 0;
		}
	}
	while(*path == '/')
		path++;
	return path;
}

void d_path(struct dentry *de, char *path)
{
	char buf[256];
	memset(buf, 0, 256);
	char *p = buf+254;
	struct dentry *tmp = de;
	if(tmp == root_dentry) {
		strcpy(path, "/");
		return;
	}
	int i, len;
	while(tmp->parent) {
		len = strlen(tmp->name);
		for(i = 0; i < len; i++) {
			*(p--) = tmp->name[len - i - 1];
		}
		*(p--) = '/';
		tmp = tmp->parent;
	}
	strcpy(path, p + 1);
}

static struct dentry *lookup(struct dentry *parent, char *name)
{
	struct dentry *de;
	struct list_t *p = parent->subdirs.next;
	while(p) {
		de = list_entry(p, struct dentry, child);
		if(!strcmp(name, de->name))
			return ddup(de);
		p = p->next;
	}
	return NULL;
}

struct dentry *namex(char *path, char *name, bool bparent)
{
	struct dentry *dp, *next;
	int off;
	if(*path == '/')
		dp = ddup(root_dentry);
	else
		dp = ddup(cpu.cur_proc->cwd);

	while((path = path_decode(path, name)) != 0) {
		if(dp->ip->type != T_DIR){
			err_info("namex ip dev:%d inum:%d type:%d isn't directory path:%s, name:%s\n", dp->ip->dev, dp->ip->inum, dp->ip->type, path, name);
			dput(dp);
			return NULL;
		}
		if(bparent && *path == 0) {
			return dp;
		}
		if(!strcmp(name, ".")) {
			next = ddup(dp);
		}
		else if(!strcmp(name, "..")) {
			next = ddup(dp->parent);
		}
		else {
			next = lookup(dp, name);
			if(!next)
				next = dp->ip->i_op->lookup(dp->ip, dp, name, &off);
			if(!next) {
				dput(dp);
				return NULL;
			}
		}
		dput(dp);
		dp = next;
	}
	if(bparent) {
		dput(dp);
		return NULL;
	}
	return dp;
}

struct dentry *namei(char *path)
{
	char name[DIR_NM_SZ];
	return namex(path, name, false);
}

struct dentry *namep(char *path, char *name)
{
	return namex(path, name, true);
}

