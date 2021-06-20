#include <stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <signal.h>

void handler(int sig);

int main(void)
{
    int state;
    int num=0;
 
    struct sigaction act;                                     /* 핸들러 구조체 변수 */
    act.sa_handler=handler;                               /* 핸들러 함수 지정*/
    sigemptyset(&act.sa_mask);                         /* sm_mask의 모든 비트를 0으로 설정 */ 
    act.sa_flags=0;                 
    state=sigaction(SIGINT, &act, 0);                  /*시그널 핸들러 등록 */
    
    if(state != 0){
        puts("sigaction() error ");
        exit(1);
    }

    while(1){
        printf("%d : 대기중 \n", num++);
        sleep(2);
        if(num>5) break;
    }
    return 0;
}

void handler(int sig)                                               /* 시그널 처리 함수 */
{
    printf("전달된 시그널은 %d \n", sig);
}

