#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void handler(int sig);
int main(int argc, char **argv)
{
    int state;
    int num=0;
    //signal(SIGINT, handler); /*인터럽트 발생시 처리*/

    while(1) 
    {
        printf("%d : 대기중 \n", num++);   
        sleep(2);
        if(num>5)
        break;
    }
    return 0;
}

/*시그널 처리 함수 */
void handler(int sig) {
    signal(SIGINT, handler);
    printf("전달된 시그널은 %d \n", sig); 
}

