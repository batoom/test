#include <stdio.h>
#include <string.h>

int main(){
	char buf[50] = "ni hao ma? hello my name is xu bao chu";

	printf("%s\n",buf);
	memcpy(buf,&buf[8],7);
	printf("%s\n",buf);

}
