#include <sys/wait.h>#include <assert.h>#include <stdio.h>#include <stdlib.h>#include <unistd.h>#include <string.h>int main(int argc, char *argv[]){    int fd[2];

	pid_t pid;

	char read_buffer[500] = {0};

	int read_count = 0;

	int status = 0;

	//创建管道

	if (pipe(fd) < 0)

	{

		printf("Create pipe failed.");

		return -1;

	}

	//创建子进程

	if ((pid = fork()) < 0)

	{

		printf("Fork failed.");

		return -1;

	}

	//子进程操作

	if (pid == 0)

	{

		printf("[child]Close read endpoint...");

		close(fd[0]);   /* 关闭不使用的读 文件描述符 */

		//复制fd[1]到标准输出

		if (fd[1] != STDOUT_FILENO)

		{

			if (dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO)

			{

				return -1;

			}

			//close fd[1]，标准输出即为fd[1]

			close(fd[1]);

		}

		//执行命令

		status = system("ls –a");

		if (status == -1)

		{

			return -1;

		}

	}

	else

	{

		printf("[parent]Close write endpoint...");

		//父进程 读 操作

		close(fd[1]);   /* 关闭不使用的写 文件描述符 */

		//从管道读数据

		read_count = read(fd[0], read_buffer, 500);

		printf("Content under current directory: \n%s", read_buffer);

	}

}
