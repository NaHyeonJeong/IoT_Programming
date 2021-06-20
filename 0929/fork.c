#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
    pid_t pid;
    int data=10;

    pid=fork();
    if(pid==-1)
        printf("fork 실패, 프로세스 id : %d \n", pid);
    printf("fork 성공, 프로세스 id : %d \n", pid);

    if(pid==0) {     /* 자식 프로세스라면 */
        data+=10;
    }
    else            /* 부모 프로세스라면 */
        data-=10;

    printf("data : %d \n", data);
    return 0;
}


