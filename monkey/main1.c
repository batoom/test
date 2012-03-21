#include <stdio.h>

#define MAX 10000
static total;
int dispatch(int m)
{
	int level;
	if((5*m+1)%4 == 0)
		level = dispatch((5*m+1)/4);
	else{
		level = 0;
		return 0;
	}
	if (level == 0)	
		total = m*5+1;
	printf("m=%d level=%d total=%d\n",m,level+1,total);
	return ++level;
}

int main()
{
	int m;
	for(m=1;m<MAX;m++)
		if(dispatch(m)>=4){
			printf("m:%d taozi:%d\n",m,total/4*5+1);
			break;
		}
}
