#include <stdio.h>


int qsort(int array[],int s,int e)
{
	int l=s;
	int r=e;
	int k=array[s];
	
	if(s>=e)
		return;

	while(r>l){
		while((r>l)&&(array[r] >= k))
			r--;
		array[l] = array[r];

		while((r>l)&&(array[l] <= k))
			l++;
		array[r] = array[l];
	}
	array[r] = k;
	printf("%d,%d\n",r,array[r]);
	qsort(array,s,r-1);
	qsort(array,r+1,e);
}

int main()
{
	int array[] = {9,4,2,5,7,8,3,5,6,0,2,4,1};
	int i;
	int len = sizeof(array)/sizeof(array[0]);
	for(i=0; i<len; i++)
		printf("%d,",array[i]);
	printf("\n");
	qsort(array,0,len);
	for(i=0; i<len; i++)
		printf("%d,",array[i]);
	printf("\n");
}
