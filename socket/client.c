#include<stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#define SERVPORT 3333
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

main(int argc, char *argv[]){
	int sockfd, recvbytes;
	char string[MAXDATASIZE];
	struct hostent *host;
	struct sockaddr_in serv_addr;
	pthread_t read_tid,write_tid;
	struct socket_data data;

	if (argc < 2) {
		fprintf(stderr,"Please enter the server's hostname! ");
		exit(1);
	}

	if((host=gethostbyname(argv[1]))==NULL) {
		herror("gethostbyname出错！");
		exit(1);
	}
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket创建出错！");
		exit(1);
	}
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SERVPORT);
	serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(serv_addr.sin_zero),8);
	if (connect(sockfd, (struct sockaddr *)&serv_addr,
	sizeof(struct sockaddr)) == -1) {
		perror("connect出错！");
		exit(1);
	}	

	strcpy(data.ipaddr,(char *)inet_ntoa(serv_addr.sin_addr));
	data.socket_fd = sockfd; 
	pthread_create(&read_tid,NULL,read_socket,(void *)&data);
	pthread_create(&write_tid,NULL,write_socket,(void *)&data);
	while(1){
		sleep(1000);
	}

	close(sockfd);

}
