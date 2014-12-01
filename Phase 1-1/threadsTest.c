#include <pthread.h>
#include <stdio.h>

void *test(void *){
	printf("look at me im a thread\n");
}

int main(){
	pthread_t sendingThread;
	if (pthread_create(&sendingThread,NULL,test,NULL))
	{
		printf("look at me" );
	}

	if(pthread_join(sendingThread,NULL)){
		printf("wtf");
	}
}