#include "stdio.h"

struct proc_info pi[64];

char *STATUS[] = {
	"UNUSED  ", 
	"EMBRYO  ",
	"READY   ",
	"RUNNING ",
	"SLEEPING",
	"ZOMBIE  ",
};

int main()
{
	memset(pi, 0, sizeof(struct proc_info) * 64);
	ps(pi, 10);
	int i = 0;
	printf("PID PPID    VSZ      STAT      CMD\n");
	for(; i < 10; i++) {
		if(pi[i].pid != 0) {
			printf(" %d   %d     %d    %s  %s\n", pi[i].pid, pi[i].ppid, 
				pi[i].vsz, STATUS[pi[i].stat], pi[i].name);
		}
	}
	return 0;
}

