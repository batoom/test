#include <stdio.h>
#include <string.h>

int char2int(char c)
{
	return c - '0';
}

char int2char(int i)
{
	return i+'0';
}

int multi(char *out, char *m1, int m2)
{
	int i;
	int c;
	int len = strlen(m1);
	char temp[65];
	int m;
	int r;

	c = 0;
	for(i=len-1;i>=0;i--){
		m = char2int(m1[i]);
		r = m*m2 + c;
		temp[i] = int2char(r%10);	
		c = r/10;
	}

	if(c != 0){
		memcpy(out+1,temp,len);
		*out = int2char(c);
		out[len+1] = '\0';
	}else{
		memcpy(out,temp,len);
		out[len] = '\0';
	}
	return 0;
}

int main(int argc,char **argv)
{
	char m1[65] = "123";
	int m2 = 2;
	char out[65];

	int i;

	for(i=0;i<argc;i++)
		printf("%s\n",argv[i]);
	
	strcpy(m1,argv[1]);
	m2 = atoi(argv[2]);
	multi(out,m1,m2);

	printf("%s\n",out);
}
