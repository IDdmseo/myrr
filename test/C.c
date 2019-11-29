#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

/* Act as a actuator */
/* 
   1. read from target
   2. execute some calcuration
   3. then, write some result to target
*/

void main(int argc, char *argv[])
{
	int i, j = 1;
    int sockfd;
    struct sockaddr_in serv_addr;
    pthread_t pid[2];

    if(argc != 3){
        perror("[Usage] ./a.out <port> <address>\n");
        exit(1);
    }

    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket errorC\n");
        exit(1);
    }

    printf("1. socket completeC\n");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[2]);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
        perror("connect errorC\n");
        exit(1);
    }

    printf("2. connect completeC\n");

	for(i=1;i<=20;i++){
		recv(sockfd, &j, sizeof(int), 0);	
		printf("[recv from target]: %d\n", j);
		j *= 2;
		send(sockfd, &j, sizeof(int), 0);
		printf("[send from C to target]: %d\n", j);
	}

    close(sockfd);
}
