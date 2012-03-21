#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>


int get_majority(int array[], int length)
{
	struct dic{
		int val;
		int count;
	};

	int i;
	int j;
	int result;
	struct dic *temp_array = (struct dic *)malloc(sizeof(struct dic)*length);
	struct dic *elemet;

	memset(temp_array,0,sizeof(struct dic)*length);
	for(i = 0; i < length; i++){
		for(j = 0; j < length; j++){
			if(array[i] == temp_array[j].val){
				temp_array[j].count++;
				break;
			}
			if(temp_array[j].val == 0){
				temp_array[j].val = array[i];
				temp_array[j].count = 1;
				break;
			}
		}
	}

	elemet = &temp_array[0];
	for(j = 0; temp_array[j].count != 0; j++){
		printf("val:%d,count:%d\n",temp_array[j].val,temp_array[j].count);
		if(elemet->count < temp_array[j].count)
			elemet = &temp_array[j];
	}
	
	printf("val:%d,count:%d\n",elemet->val,elemet->count);
	result = elemet->val;
	free(temp_array);
	return result;
}
int get_hash_majority(int array[], int length)
{
	struct dic{
		int val;
		int count;
	};

	int i;
	int j;
	int result;
	struct dic *temp_array = (struct dic *)malloc(sizeof(struct dic)*length);
	struct dic *elemet;
	int hash_key;

	memset(temp_array,0,sizeof(struct dic)*length);
	for(i = 0; i < length; i++){
		hash_key = array[i] & 0x0f;
conflict:		
		if(temp_array[hash_key].count == 0){
			temp_array[hash_key].val = array[i];
			temp_array[hash_key].count = 1;
		}else{
			if(array[i] == temp_array[hash_key].val)
				temp_array[hash_key].count++;
			else{
				if(hash_key < 0x10)
					hash_key = 0x10;
				else
					hash_key += 1;

				goto conflict;
			}
		}
		
	}

	elemet = &temp_array[0];
	for(j = 0; j < length; j++){
		printf("val:%d,count:%d\n",temp_array[j].val,temp_array[j].count);
		if(elemet->count < temp_array[j].count)
			elemet = &temp_array[j];
	}
	
	printf("val:%d,count:%d\n",elemet->val,elemet->count);
	result = elemet->val;
	free(temp_array);
	return result;
}
int hash_majority(int array[], int length)
{
	struct dic{
		int val;
		int count;
	};

	int i;
	int j;
	int result;
	struct dic *temp_array = (struct dic *)malloc(sizeof(struct dic)*length);
	struct dic *elemet;
	int hash_key;

	memset(temp_array,0,sizeof(struct dic)*length);
	for(i = 0; i < length; i++){
		hash_key = array[i] & 0x0f;
collision:
		if((temp_array[hash_key].val == 0) || (temp_array[hash_key].val == array[i])){
			temp_array[hash_key].val = array[i]; 
			temp_array[hash_key].count += 1;
		}else{
			if(hash_key < 0x10)
				hash_key = 0x10;
			else
				hash_key += 1;
			goto collision;
		}
	}

	elemet = &temp_array[0];
	for(j = 0; j < length; j++){
		printf("val:%d,count:%d\n",temp_array[j].val,temp_array[j].count);
		if(elemet->count < temp_array[j].count)
			elemet = &temp_array[j];
	}
	
	printf("val:%d,count:%d\n",elemet->val,elemet->count);
	result = elemet->val;
	free(temp_array);
	return result;
}


int hash_majority_e(int array[], int length)
{
	struct dic{
		int val;
		int count;
	};

	int i;
	int j;
	int result;
	struct dic *temp_array = (struct dic *)malloc(sizeof(struct dic)*length);
	struct dic *elemet;
	int hash_key;

	memset(temp_array,0,sizeof(struct dic)*length);
	for(i = 0; i < length; i++){
		hash_key = array[i] & 0x0f;

		while((temp_array[hash_key].val != 0) && (temp_array[hash_key].val != array[i])){
			if(hash_key < 0x10)
				hash_key = 0x10;
			else
				hash_key += 1;
		}
		temp_array[hash_key].val = array[i]; 
		temp_array[hash_key].count += 1;
	}

	elemet = &temp_array[0];
	for(j = 0; j < length; j++){
		printf("val:%d,count:%d\n",temp_array[j].val,temp_array[j].count);
		if(elemet->count < temp_array[j].count)
			elemet = &temp_array[j];
	}
	
	printf("val:%d,count:%d\n",elemet->val,elemet->count);
	result = elemet->val;
	free(temp_array);
	return result;
}


int main()
{
	int array[] = {279,247,263,263,263,263,264,263,264,263,263,263,263,264,263,263,263,263,264,262,264,262,264,264,264,264,264,264,264,264,263,263};

	int result = hash_majority_e(array,sizeof(array)/sizeof(array[0]));
	int result2 = get_hash_majority(array,sizeof(array)/sizeof(array[0]));
	printf("the majority val:%d,%d\n",result,result2);
}



