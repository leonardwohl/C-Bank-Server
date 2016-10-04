#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <resolv.h>

#define MAX_INT 2147483647 

void sendMes(int* clientSocket){
	
	char buff[1024];
	//char buff2[1024];
	
	for(;;){
		
		memset(buff, 0, sizeof(buff));
		//memset(buff2, 0, sizeof(buff2));
		scanf("%s", buff);
		send(*clientSocket, buff, 1024, 0);
		if(strcmp(buff, "open") == 0){
			fgets(buff, MAX_INT, stdin);
			send(*clientSocket, buff, 1024, 0);
		}else if(strcmp(buff, "start") ==  0){
			fgets(buff, MAX_INT, stdin);
			send(*clientSocket, buff, 1024, 0);
		}
		if(strcmp(buff, "exit") == 0) return;
		sleep(2);
	}
	
}

void recMes(int* serverAddr){
	
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	while(recv(*serverAddr, buff, 1024, 0) > 0){
		
		printf("%s\n",buff); 
		if(strcmp(buff, "exit") == 0) return;
		
		memset(buff, 0, sizeof(buff));
	}
	printf("Disconnect from server\n");
	return;
}
int main(int argc, char** argv){
	
	
	int clientSocket;
	struct sockaddr_in serverAddr;
	socklen_t addr_size;
	int connected = 0;
	
	if(argv[1] == NULL){
		
		printf("Enter an address, exiting\n");
		exit(0);
		
	}
	
	while(connected == 0){
		clientSocket = socket(PF_INET, SOCK_STREAM, 0);
		  
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(9999);
		serverAddr.sin_addr.s_addr = inet_addr(argv[1]); 
		memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  
		
		addr_size = sizeof serverAddr;
		if(connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size) != 0){
			
				printf("Couldn't find server, trying again in 3 seconds...\n");
				sleep(3);
		}else {
			connected = 1;
		}

	}
	int* arg = &clientSocket;	
	
	void* sendM = (void*(*)(void*))sendMes;
	
	pthread_t* sender = malloc(sizeof(pthread_t));
	pthread_create(sender, NULL, sendM, (void*)arg);
	
	void* recM = (void*(*)(void*))recMes;
	
	pthread_t* reciever = malloc(sizeof(pthread_t));
	pthread_create(sender, NULL, recM, (void*)arg);
	
	pthread_join(*sender, NULL);
	pthread_join(*reciever, NULL);
	
	return 0;
}
