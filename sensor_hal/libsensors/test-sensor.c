#include "sensors.c"

int main()
{
	int fd = sensd_open();

	printf("fd is %d, %s\n", fd, strerror(errno));

	FILE* fp = fdopen(fd, "r+");
	setlinebuf(fp);

	int ret = fprintf(fp, "list-sensors\n");
	printf("hello ret %d %s, errno %d, fileno %d\n", ret, strerror(errno), errno, fileno(fp));
	char buff[1024];
	fprintf(fp, "start\n");
	while (fgets(buff, 1024, fp)) {
		printf("%s", buff);
	}
	return 0;
}
