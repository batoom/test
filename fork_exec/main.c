#include <stdio.h>

int main()
{
	int pid;

	pid = fork();
	if(pid == 0){
		execv("./exec",NULL);
		printf("hello sub process\n");
	}else{
		printf("hello main process\n");
	}
}
