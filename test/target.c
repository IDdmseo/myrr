#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

/*
   1. read data from A node.
   2. write data to C node.
   3. read data from C node.
*/

void thread_handlerB(int *arg);
void thread_handlerC(int *arg);
void thread_handlerC2(int *arg);

int i = 1, j = 1, k = 1;
int cnt = 0;

void main(int argc, char *argv[])
{
	int servfd, clntfd[2], clnt_len, idx = 0;
	struct sockaddr_in serv_addr, clnt_addr;
	pthread_t pid[3];

	if((servfd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket error\n");
		exit(1);
	}

	printf("1. socket complete\n");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if(bind(servfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
		perror("bind error\n");
		exit(1);
	}

	printf("2. bind complete\n");

	while(idx < 2){
		if(listen(servfd, 5) == -1){
			perror("listen error\n");
			exit(1);
		}

		printf("3. listen complete\n");
		
		clnt_len = sizeof(clnt_addr);
		
		if((clntfd[cnt] = accept(servfd, (struct sockaddr *)&clnt_addr, &clnt_len)) == -1){
			perror("accept error\n");
			exit(1);
		}

		printf("4. accept complete\n");	
		if(idx == 0)
			pthread_create(&pid[0], NULL, (void *)&thread_handlerB, (int *)&clntfd[cnt]);
		else if(idx == 1)
			pthread_create(&pid[1], NULL, (void *)&thread_handlerC, (int *)&clntfd[cnt]);
			pthread_create(&pid[2], NULL, (void *)&thread_handlerC2, (int *)&clntfd[cnt]);
	
		idx += 1;
		cnt += 1;
	}

	pthread_join(pid[0], NULL);
	pthread_join(pid[1], NULL);
	
	close(clntfd[0]);
	close(clntfd[1]);
}

void thread_handlerB(int *arg)
{
	int fd = *(int *)arg;

	for(i=0;i<20;i++){
		recv(fd, &j, sizeof(int), 0);
//		printf("[recv from B]: %d\n", j);
	}
}

void thread_handlerC(int *arg)
{
	int fd = *(int *)arg;

	for(i=0;i<20;i++){
		k = i + j;
		send(fd, &k, sizeof(int), 0);	
//		printf("[send from target to C]: %d\n", k);
	}
	
}

void thread_handlerC2(int *arg)
{
	int fd = *(int *)arg;

	for(i=0;i<20;i++){
		recv(fd, &j, sizeof(int), 0);	
//		printf("[recv from C]: %d\n", j);
	}
	
}
