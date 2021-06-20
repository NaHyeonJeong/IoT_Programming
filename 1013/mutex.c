#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
void * thread_increment (void *arg);
char thread1[] = "A Thread";
char thread2[] = "B Thread";
pthread_mutex_t mutx;
int number=0;
int main(int argc, char **argv)
{
	pthread_t t1, t2;
	void *thread_result;
	int state;
	state = pthread_mutex_init(&mutx, NULL);
	if(state){
		puts("뮤텍스 초기화 실패");
		exit(1);
		
	}
	pthread_create(&t1, NULL, thread_increment, &thread1);  
	pthread_create(&t2, NULL, thread_increment, &thread2);
	pthread_join(t1, &thread_result);
	pthread_join(t2, &thread_result);
	printf("최종 number : %d \n", number);
	pthread_mutex_destroy(&mutx);
	return 0;
}
void * thread_increment(void * arg){
	int i;
	for(i=0; i<5; i++) {
		pthread_mutex_lock(&mutx);
		sleep(1);
		number++;
		printf("실행 : %s,  number : %d \n", (char*)arg, number);
		pthread_mutex_unlock(&mutx);
	}
}
