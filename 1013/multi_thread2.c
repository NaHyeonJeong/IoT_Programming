#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>

void *thread_function(void *arg);
int main(int argc, char **argv)
{
	int state;
	pthread_t t_id; 
	void * t_return;
	state = pthread_create(&t_id, NULL, thread_function, NULL); /* 쓰레드 생성 */
	if(state != 0){
		puts("쓰레드 생성 오류");
		exit(1);
	}
	printf("생성된 쓰레드의 ID : %ld \n", t_id);
	/* 쓰레드 종료 시까지 main함수의 실행을 지연 */
	state = pthread_join(t_id, &t_return); /* 리턴 값 저장 */

	if(state != 0){
		puts("쓰레드 Join 오류");
		exit(1);
	}
	printf("main함수 종료, 쓰레드 리턴 %s", (char*)t_return);
	free(t_return);
	return 0;
}
void *thread_function(void *arg){
	int i;
	char * p = (char*)malloc(20*sizeof(char));
	strcpy(p, "쓰레드 종료 됨!\n");
	for(i=0; i<3; i++){
		sleep(2);
		puts("쓰레드 실행중");
	}
	return p;
}	

