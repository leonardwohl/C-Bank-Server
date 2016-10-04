#include <pthread.h>
struct Account{

	char name[100];
	float balance;
	pthread_mutex_t* use;
	int inUse;
	int init;

};
