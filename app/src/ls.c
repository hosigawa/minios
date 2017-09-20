#include "stdio.h"
#include "unistd.h"

static char *MONS[] = {
	"Jan", 
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};

bool bdetails = false;
bool bhide = true;

void print_file(char *path, char *name, struct file_stat *st)
{
	struct time_v tm;
	localtime(st->mtime, &tm);
	if(st->type == T_LINK) {
		char link[64];
		memset(link, 0, 64);
		int fd = open(path ? path:name, 0);
		if(fd < 0)
			link[0] = '?';
		else {
			read(fd, link, 64);
			close(fd);
		}
		printf("%s  %3d  %6d  %s %2d  %02d:%02d  %s -> %s\n", "l   ", st->nlink, 
												st->size, MONS[tm.mon - 1], tm.day, tm.h, tm.m, name, link);
	}
	else {
		printf("%s  %3d  %6d  %s %2d  %02d:%02d  %s\n", (st->type == T_DIR) ? "d   " : (st->type == T_DEV) ? "C   " : "-   ", st->nlink, 
												st->size, MONS[tm.mon - 1], tm.day, tm.h, tm.m, name);
	}
}

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
	if(bdetails){
		printf("TYPE LINK    SIZE  DATE    TIME   NAME\n");
		printf("--------------------------------------\n");
	}
	if(stat.type == T_DIR) {
		int sfd;
		char name[64];
		struct dirent de;
		memset(&de, 0, sizeof(de));
		int ret = 0;
		while((ret = readdir(fd, &de)) >= 0) {
			if(bhide && de.name[0] == '.'){
				memset(&de, 0, sizeof(de));
				continue;
			}
			if(bdetails) {
				memset(name, 0, 64);
				if(path[strlen(path) - 1] == '/')
					sprintf(name, "%s%s", path, de.name);
				else
					sprintf(name, "%s/%s", path, de.name);
				sfd = open(name, 0);
				if(sfd < 0){
					continue;
				}
				if(fstat(sfd, &sub) < 0){
					close(sfd);
					continue;
				}
				print_file(name, de.name, &sub);
				close(sfd);
			}
			else {
				printf("%s ", de.name);
			}
			memset(&de, 0, sizeof(de));
		}
		if(!bdetails)
			printf("\n");
	}
	else {
		print_file(NULL, path, &stat);
	}
	close(fd);
}

int get_opt(char *arg)
{
	while(*arg) {
		if(*arg == '-'){
			arg++;
			continue;
		}
		if(*arg == 'l')
			bdetails = true;
		else if(*arg == 'a')
			bhide = false;
		else {
			printf("ls: %s: unknow option\n", arg);
			return -1;
		}
		arg++;
	}
	return 0;
}

int main(int argc, char** argv)
{
	char *path = ".";
	int i;
	for(i = 1; i < argc; i++) {
		switch(argv[i][0]){
			case '-':
				if(get_opt(argv[i]) < 0)
					return -1;
				break;
			default:
				path = argv[i];
				break;
		}
	}

	ls(path);
	return 0;
}

