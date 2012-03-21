#include <stdio.h>

#define dbg_msg(fmt, ...)                                    \
	printf("xu# %s " fmt "\n", \
	        __func__, ##__VA_ARGS__)

int main()
{
	printf("test\n");
	dbg_msg("dbgmsg");
}
