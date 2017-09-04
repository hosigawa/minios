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

void print_file(char *path, struct file_stat *st)
{
	struct time_v tm;
	localtime(st->mtime, &tm);
	printf("%s  %3d  %6d  %s %2d %02d:%02d  %s\n", (st->type == T_DIR) ? "d   " : (st->type == T_DEV) ? "C   " : "-   ", st->nlink, 
												st->size, MONS[tm.mon - 1], tm.day, tm.h, tm.m, path);
}

void fmt_name(char *name, char *path, char *de)
{
	memset(name, 0, 64);
	int len = strlen(path);
	memmove(name, path, len);
	if(name[len-1] != '/')
		memmove(name+len, "/", 1);
	memmove(name+len+1, de, strlen(de));
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
	struct dirent de;
	int ret;
	if(bdetails){
		printf("TYPE LINK    SIZE  DATE   TIME   NAME\n");
		printf("-------------------------------------\n");
	}
	if(stat.type == T_DIR) {
		int sfd;
		char name[64];
		while((ret = read(fd, (char *)&de, sizeof(de))) == sizeof(de)) {
			if(de.inum == 0)
				continue;
			if(bhide && de.name[0] == '.')
				continue;
			if(bdetails) {
				fmt_name(name, path, de.name);
				sfd = open(name, 0);
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
		if(!bdetails)
			printf("\n");
	}
	else {
		print_file(path, &stat);
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

