#include "unistd.h"
#include "malloc.h"
#include "usyscall.h"
#include "stdio.h"

int exec(char *path, char **argv, char **envp)
{
	int ret = execv(path, argv, envp);
	if(ret == -1){
		char full_path[64];
		int envc = 0;
		while(envp[envc]) {
			memset(full_path, 0, 64);
			sprintf(full_path, "%s/%s", envp[envc++], path);
			ret = execv(full_path, argv, envp);
			if(ret == -1)
				continue;
			else
				return ret;
		}		
		return ret;
	}

	return ret;
}

