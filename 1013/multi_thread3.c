#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
void *thread_summation(void *arg);
int sum=0; // 임계 구역
int sum1[]={1, 5};
int sum2[]={6, 10};

int main(int argc, char **argv) 
{
	pthread_t id_t1, id_t2;
	void * t_return;
	pthread_create(&id_t1, NULL, thread_summation, (void *)sum1);
	pthread_create(&id_t2, NULL, thread_summation, (void *)sum2);
	/* 쓰레드 종료 시까지 main함수의 실행을 지연 */
    pthread_join(id_t1, &t_return);
	pthread_join(id_t2, &t_return);
	
	printf("main함수 종료, sum = %d \n", sum);
	return 0;
}
void *thread_summation(void *arg) {
	int start = ((int*)arg)[0];
	int end = ((int*)arg)[1];
	for( ; start<=end; start++)  {
		sum+=start;
	}
}
