#include<stdio.h> 
#include<stdlib.h> 
#include<string.h> 
#include<unistd.h> 
#include<arpa/inet.h> 
#include<sys/types.h> 
#include<sys/socket.h>
void error_handling(char *message);

int main(int argc, char **argv)
{
    int serv_sock[3];
    int i;
    
    for(i=0;i<3;i++) {
        //소켓 생성 
        serv_sock[i]=socket(PF_INET,SOCK_STREAM,0);
        if(serv_sock[i]==-1)
            error_handling("socket() error");
        printf("%d 번째 서버 소켓의 디스크립터 값 %d \n", i+1,serv_sock[i]);
    }
}

void error_handling(char *message) {
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}
