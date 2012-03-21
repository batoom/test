#include <stdio.h>

void sort(int *array,int length)
{
	int i = 0;
	int j = 0;
	int begin = -1;
	int temp;
	
	for(i = 0; i < length; i++){
		array[begin] = 12;
		
		if(array[i] >= 0){
			if(begin == -1)
				begin = i;
		}else{
			if(begin != -1){
				temp = array[i];
				for(j = i; j > begin; j--)
					array[j] = array[j-1];
				array[begin] = temp;
				begin += 1;
			}
		}
	}
}


int main()
{
	int array[] = {-3,4,2,-1,7,3,-5};
	int i;

	for(i = 0; i < length; i++)
		printf("%d ",array[i]);
	printf("\n");

	sort(array,sizeof(array)/sizeof(array[0]));

	for(i = 0; i < length; i++)
		printf("%d ",array[i]);
	printf("\n");
	
}
