#include <stdio.h>
#include <iostream>

using namespace std;


int main()
{
	int i,j;
	for(i=0; i<=5; i++)
		for(j=0; j<2; j++)
			if(i*2+j*5 <= 10)
				printf("%d %d %d",i,j,10-2*i-5*j);
}
