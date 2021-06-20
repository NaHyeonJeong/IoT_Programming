#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/time.h>

#define BUFSIZE 30
int main(int argc, char  **argv)
{
     fd_set reads, temps;
     int result;

     char message[BUFSIZE];
     int str_len;
     struct timeval timeout ;

     FD_ZERO(&reads);
     FD_SET(0, &reads); /* standard input 설정
     /*
     timeout.tv_sec = 5;
     timeout.tv_usec = 100000;
     */ 
     /* 이곳에 설정할 경우 문제 발생*/

     while(1)
     {
        temps = reads;
        timeout.tv_sec=5;
        timeout.tv_usec=0;

        result=select(1, &temps, 0,0, &timeout);
        if(result==-1){ /*select  함수 오류 발생 */
           puts("select 오류 발생");
           exit(1);
        }
        else if (result==0){ /* time-out에 의한 리턴 */
           puts("시간이 초과되었습니다 : ");
        }
        else { /* 파일 디스크립터 변화에 의한 리턴 */
           if(FD_ISSET(0, &temps)) {
              str_len = read(0, message, BUFSIZE);
              message[str_len]=0;
              printf("message from console : %s\n", message);
            }
        }
    }  /*while(1) end */
} 




