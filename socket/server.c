#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <pthread.h>

#define SERVPORT 3333 /*服务器监听端口号 */
#define BACKLOG 10 /* 最大同时连接请求数 */
#define MAXDATASIZE 256 /*每次最大数据传输量 */


struct socket_data{
	int socket_fd;
	char ipaddr[20];
	
};

void *read_socket(void *arg)
{
	struct socket_data * data = (struct socket_data *)arg;
	int client_fd = data->socket_fd;
	char string[MAXDATASIZE];
	int recvbytes;

	while(1){
		if ((recvbytes=recv(client_fd, string, MAXDATASIZE, 0)) <= 0) {
			close(client_fd);
			perror("recv error！");
			exit(1);
		}
		string[recvbytes] = '\0';
		printf("%s: %s\n",data->ipaddr,string);	
	}
}

void *write_socket(void *arg)
{
	struct socket_data * data = (struct socket_data *)arg;
	int client_fd = data->socket_fd;
	char string[MAXDATASIZE];
	int recvbytes;

	while(1){
		fgets(string,MAXDATASIZE,stdin);
		printf("\n");
		if(strcmp(string,"q") == 0){
			close(client_fd);
			break;
		}
		if(send(client_fd,string,strlen(string),0) == -1){
			perror("send error");
			close(client_fd);
			exit(0);
		}
	}
}

main()
{
	int sockfd,client_fd; /*sock_fd：监听socket；client_fd：数据传输socket */
	struct sockaddr_in my_addr; /* 本机地址信息 */
	struct sockaddr_in remote_addr; /* 客户端地址信息 */
	int sin_size;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("socke"); exit(1);
	}
	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(SERVPORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero),8);
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
	perror("bind出错！");
	exit(1);
	}
     if (listen(sockfd, BACKLOG) == -1) {
		perror("listen出错！");
		exit(1);
     }
     while(1) {
		sin_size = sizeof(struct sockaddr_in);
		if ((client_fd = accept(sockfd, (struct sockaddr *)&remote_addr, &sin_size)) == -1) {
			perror("accept出错");
			continue;
		}
		printf("received a connection from %s \n", (char *)inet_ntoa(remote_addr.sin_addr));
		if (!fork()) { /* 子进程代码段 */
			pthread_t read_tid,write_tid;
			struct socket_data data;
			strcpy(data.ipaddr,(char *)inet_ntoa(remote_addr.sin_addr));
			data.socket_fd = client_fd; 
			pthread_create(&read_tid,NULL,read_socket,(void *)&data);
			pthread_create(&write_tid,NULL,write_socket,(void *)&data);
			while(1){
				sleep(1000);
			}
         }         
     }
}
