#include <stdio.h>

#define MAX_INTEAGE 1000

int main()
{
	int a,b,c;

	for(c=0;c<MAX_INTEAGE;c++)
		for(b=c;b<MAX_INTEAGE;b++)
			for(a=b;a<MAX_INTEAGE;a++)
				if((3*a+2-5*b == 0) && (7*c+4-5*b == 0)) {
					printf("%d %d %d %d\n",a,b,c,5*b);
					break;
				}
}
