#include <stdio.h>

#define max_t(type, x, y) ({			\
	type __max1 = (x);			\
	type __max2 = (y);			\
	__max1 > __max2 ? __max1: __max2; })


int main()
{
	int a,b;
	int result;	

	printf("please input number a:");
	scanf("%d",&a);
	printf("please input number b:");
	scanf("%d",&b);
	if(1)
		result = max_t(int,a,b);
	else
		printf("else\n");

	printf("the max number: %d\n",result);

}
