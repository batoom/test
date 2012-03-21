#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc<2){
		printf("please enter a num prameter\n");
		return 0;
	}
		
	char a=(char)atoi(argv[1]);
	printf("%x,%i\n",(char)a,a);
}


