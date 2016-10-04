#include "account.h"
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

#define MY_PORT		9999
#define MAXBUF		1024

void status_printer(struct Account* list){

	int j;
	int s;
	for(;;){
		pthread_mutex_lock(list[20].use);
		printf("\nBank Server Status:\n");
		for(j=0, s=0; j<20; j++){
			if(list[j].init){
				printf("%s : %f", list[j].name, list[j].balance);
				s++;
				if(list[j].inUse){
					
					printf(" : IN SERVICE\n");
			
				}else{
					printf("\n");
				}
			}else{
				//printf("UNUSED\n");
			}
		}
		if (s == 0){
			printf("No accounts have been made");
		}
		printf("\n");
		pthread_mutex_unlock(list[20].use);
		sleep(20);
	}
}

void client_acceptor(int* clientfd){
	
	static int initialized = 0;
	static int accountnum;
	static pthread_t* server_stat;
	static pthread_mutex_t* openlock;
	static struct Account* accountlist;
	int i;
	int e;
	float f;
	struct Account* active = NULL;
	
	if(!initialized){
		initialized = 1;
		accountlist = malloc(sizeof(struct Account)*21);
		accountnum = 0;
		server_stat =  malloc(sizeof(pthread_t));
		accountlist[20].use = malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(accountlist[20].use, NULL);
		openlock = malloc(sizeof(pthread_mutex_t));
		openlock = accountlist[20].use;
		void* print = (void*(*)(void*))status_printer;
		pthread_create(server_stat, NULL, print, (void*)accountlist);
		
	}
	send(*clientfd, "Welcome! Enter a request.", MAXBUF, 0);
	char buffer[MAXBUF];
	while(recv(*clientfd, buffer, 1024, 0)>0){
		
		e=0;
		if(strcmp(buffer, "exit") == 0){
			send(*clientfd, "GOODBYE", MAXBUF, 0);
			close(*clientfd);
			return;
		}else if(strcmp(buffer, "open") == 0){
			
			memset(buffer, 0, sizeof(buffer));
			recv(*clientfd, buffer, 1024, 0);
			pthread_mutex_lock(openlock);
			if(accountnum <20){
				
				for(i = 0; i<20 && e == 0; i++){
					if(strcmp(accountlist[i].name, buffer) == 0){
						
						send(*clientfd, "There is already an account by that name, did you mean 'start'?", MAXBUF, 0);
						e = 1;
					}
				}
				if(e == 0){
					
					send(*clientfd, "Account Opened", MAXBUF, 0);
					strcpy(accountlist[accountnum].name, buffer);
					accountlist[accountnum].balance = 0.0;
					accountlist[accountnum].init = 1;
					accountlist[accountnum].use = malloc(sizeof(pthread_mutex_t));
					pthread_mutex_init(accountlist[accountnum].use, NULL);
					accountnum++;
				}
			}
			pthread_mutex_unlock(openlock);
			
		}else if(strcmp(buffer, "start") == 0){
			memset(buffer, 0, sizeof(buffer));
			recv(*clientfd, buffer, 1024, 0);
			for(i = 0; i<20; i++){
				if(strcmp(accountlist[i].name, buffer) == 0){
					
					while(accountlist[i].inUse == 1){
						send(*clientfd, "Account in use, trying again in 2 seconds...", MAXBUF, 0);
						sleep(2);
					}
					
						
					accountlist[i].inUse = 1;
					pthread_mutex_lock(accountlist[i].use);
					active = &accountlist[i];
					send(*clientfd, "Logged In", MAXBUF, 0);
					continue;
					
				}
				
			}
			
			if(active == NULL && e == 0){
				
				send(*clientfd, "No account by that name found", MAXBUF, 0);
				e = 1;
				
			}
			
			while(e == 0){
				
				memset(buffer, 0, sizeof(buffer));
				recv(*clientfd, buffer, 1024, 0);
			
				if(strcmp(buffer, "balance")==0){
					memset(buffer, 0, sizeof(buffer));
					sprintf(buffer, "Balance : %f", active->balance);
					send(*clientfd, buffer, MAXBUF, 0);
					
				}else if(strcmp(buffer, "finish") == 0){
					
					e = 1;
					active->inUse = 0;
					pthread_mutex_unlock(active->use);
					active = NULL;
					send(*clientfd, "Logging Out", MAXBUF, 0);
					
				}else if(strcmp(buffer, "credit") == 0){
					memset(buffer, 0, sizeof(buffer));
					recv(*clientfd, buffer, 1024, 0);
					f = strtod(buffer, NULL);
					if(f >= 0){
						
						active->balance += f;
						send(*clientfd, "Balance Updated", MAXBUF, 0);
					}else{
						send(*clientfd, "Bad Value", MAXBUF, 0);
					}
				}else if(strcmp(buffer, "debit") == 0){
					
					memset(buffer, 0, sizeof(buffer));
					recv(*clientfd, buffer, 1024, 0);
					f = strtod(buffer, NULL);
					if(f >= 0 && f <= active->balance){
						
						active->balance -= f;
						send(*clientfd, "Balance Updated", MAXBUF, 0);
					}else{
						send(*clientfd, "Bad Value", MAXBUF, 0);
					}
				}else {
					
					send(*clientfd, "Unrecognized request", MAXBUF, 0);
					
				}
			}
			
		}else {
			send(*clientfd, "Unrecognized request", MAXBUF, 0);
		}
		
		memset(buffer, 0, sizeof(buffer));	
	}
	printf("Client Disconnect from server!");
	return;
}

int main(int argc, char **argv) {   
	int sockfd;
	int test;
	struct sockaddr_in self;
	//char buffer[MAXBUF];
	
	test=1;
	/*---Create streaming socket---*/
    if ( (sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("Socket");
		_exit(errno);
	}

	/*---Initialize address/port structure---*/
	memset(&self, 0, sizeof(self));
	self.sin_family = AF_INET;
	self.sin_port = htons(MY_PORT);
	self.sin_addr.s_addr = INADDR_ANY;
	
	//printf("initialized at %d : %d\n", self.sin_addr.s_addr, self.sin_port);
	test = 2;
	/*---Assign a port number to the socket---*/
    if ( bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0 )
	{
		perror("socket--bind");
		_exit(errno);
	}

	/*---Make it a "listening socket"---*/
	if (listen(sockfd, 20) != 0){
		perror("socket--listen");
		_exit(errno);
	}
	printf("Server is awaiting clients.\n");
	for(;;){	
		int* c = malloc(sizeof(int));
		
		struct sockaddr_in client_addr;
		unsigned int addrlen=sizeof(client_addr);

		
		*c = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
		printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		
		pthread_t* acc = malloc(sizeof(pthread_t));
	
		
		void* accept = (void*(*)(void*))client_acceptor;
		
		pthread_create(acc, NULL, accept, (void*)c);
		
		
	}

	/*---Clean up (should never get here!)---*/
	close(sockfd);
	return 0;
}
