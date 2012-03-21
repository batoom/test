#include <stdio.h>

struct judge_t 
{
	int total_score;
	int num;
};

int cal_score(int score[],int judge_type[],int n)
{
	int i;
	int result;
	struct judge_t spec_judge = {0};
	struct judge_t norm_judge = {0};

	for(i = 0; i < n; i++){
		if(judge_type[i] == 1){
			spec_judge.total_score += score[i];
			spec_judge.num++;
		}else{
			norm_judge.total_score += score[i];
			norm_judge.num++;
		}
	}

	if(norm_judge.num != 0){
		result = spec_judge.total_score / spec_judge.num * 6 / 10 +
		     norm_judge.total_score / norm_judge.num * 4 / 10;			     
	}else{
		result = spec_judge.total_score / spec_judge.num;
	}

	return result;
}


int main()
{
	int score[] = {7,8,9};
	int judge[] = {1,2,1};
	
	printf("result = %d\n",cal_score(score,judge,sizeof(score)/sizeof(int)));

}
