#include <stdio.h>

char* revert(char *to,char *from)
{
	if(*from != '\0'){
		to = revert(to,from+1);
		*to++ = *from;
		*to = '\0';
	}
	return to;
}
int main()
{
	char from[12] = "12345678";
	char to[12];

	revert(to,from);

	printf("%s\n%s\n",from,to);
}
