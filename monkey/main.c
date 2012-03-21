#include <stdio.h>

#define MAX 10000


int dispach_peach(int num)
{
	int level;
	if(num == num/5*5+1)
		level = dispach_peach(num - num/5 -1);
	else{
		level = 0;
		return 0;
	}
	return ++level;
}

int main()
{
	int i;
	for(i = 0; i < MAX; i++)
		if(dispach_peach(i) >= 5){
			printf("%d\n",i);
			break;
		}

}
