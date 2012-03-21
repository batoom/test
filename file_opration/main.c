#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

int main()
{
	int fd;
	umask(0);
	fd = creat("hello",0666);
	if(fd != -1){
		close(fd);
	}else{
		printf("creat fail\n");
	}
	
}
