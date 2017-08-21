#include "libc.h"

void ls(char *path) 
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
	if(stat.type == T_FILE) {
		printf("%s  %s  %d  %d\n", path, (sub.type == T_DIR) ? "D" : (sub.type == T_DEV) ? "DEV" : "F", sub.nlink, sub.size);
	}
	else if(stat.type == T_DIR) {
		int sfd;
		while((ret = read(fd, (char *)&de, sizeof(de))) == sizeof(de)) {
			if(de.inum == 0)
				continue;
			sfd = open(de.name, 0);
			if(sfd < 0)
				continue;
			if(fstat(sfd, &sub) < 0){
				close(sfd);
				continue;
			}
			printf("%s  %s  %d  %d\n", de.name, (sub.type == T_DIR) ? "D" : (sub.type == T_DEV) ? "DEV" : "F", sub.nlink, sub.size);
			close(sfd);
		}
	}
	else if(stat.type == T_DEV) {
		printf("%s  %s  %d  %d\n", path, (sub.type == T_DIR) ? "D" : (sub.type == T_DEV) ? "DEV" : "F", sub.nlink, sub.size);
	}
	close(fd);
}

int main()
{
	ls(".");
	return 0;
}
