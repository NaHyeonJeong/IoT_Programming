#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

void * thread_snd(void *arg);
void * thread_rcv(void *arg);
sem_t bin_sem, bin_sem2;
int number=0;
char thread1[]="A Thread";
char thread2[]="B Thread";
char thread3[]="C Thread";
int main(int argc, char **argv)
{  
    pthread_t t1, t2, t3;
    void *thread_result;  
    int state, state2;  
    state = sem_init(&bin_sem, 0, 0); /* bin_sem은 0으로 설정 */
    state2 = sem_init(&bin_sem, 0, 0);
    if(state != 0 || state2 != 0) {   
        puts("세마포어 초기화 실패");    
        exit(1);
    }

    pthread_create(&t1, NULL, thread_snd, &thread1);
    pthread_create(&t2, NULL, thread_rcv, &thread2); 
    pthread_create(&t3, NULL, thread_rcv, &thread3);  
    pthread_join(t1, &thread_result);
    pthread_join(t2, &thread_result);
    pthread_join(t3, &thread_result);
    printf("최종 number : %d \n", number);
    sem_destroy(&bin_sem);
    return 0;
}
void * thread_snd(void * arg)
{
    int i;
    for(i=0; i<4; i++){
        /*      while(number != 0) 
                 sleep(1);      */

        number++;
        printf("실행 : %s,  number : %d \n", (char*)arg, number);
        sem_post(&bin_sem);
        sem_wait(&bin_sem2);
    }
}
void * thread_rcv(void * arg)
{
    int i;
    for(i=0; i<2; i++){
        sem_wait(&bin_sem);
        number--;
        printf("실행 : %s, number : %d \n", (char*)arg, number);
        sem_post(&bin_sem2);
    }
}


