#include "libc.h"

void print_file(char *path, struct file_stat *st)
{
	printf("%s  %s  %d  %d\n", path, (st->type == T_DIR) ? "D" : (st->type == T_DEV) ? "DEV" : "F", st->nlink, st->size);
}

void ls(char *path, bool details) 
{
	int fd;
	fd = open(path, 0);
	if(fd < 0) {
		printf("ls: cannot access %s: No such file or directory\n", path);
		return;
	}
	struct file_stat stat, sub;
	if(fstat(fd, &stat) < 0) {
		printf("ls: fstat error\n");
		close(fd);
		return;
	}
	struct dirent de;
	int ret;
	printf("name    type    link    size\n");
	if(stat.type == T_DIR) {
		int sfd;
		while((ret = read(fd, (char *)&de, sizeof(de))) == sizeof(de)) {
			if(de.inum == 0)
				continue;
			if(details) {
				sfd = open(de.name, 0);
				if(sfd < 0)
					continue;
				if(fstat(sfd, &sub) < 0){
					close(sfd);
					continue;
				}
				print_file(de.name, &sub);
				close(sfd);
			}
			else {
				printf("%s ", de.name);
			}
		}
		if(!details)
			printf("\n");
	}
	else {
		print_file(path, &stat);
	}
	close(fd);
}

int main(int argc, char** argv)
{
	if(argc == 1)
		ls(".", false);
	else if(argc == 2){
		if(argv[1][0] == '-'){
			if(argv[1][1] == 'l')
			ls(".", true);
		}
		else
			ls(argv[1], false);
	}
	else {
	}
	return 0;
}
