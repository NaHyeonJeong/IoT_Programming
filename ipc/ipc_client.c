/*
프로그램 : IPC 서버 구축 - Guess Word - Server
작 성 자 : 나현정(201858042)
작 성 일 : 2020.10.20
설    명 : 클라이언트는 서버(자식 프로세스)에서 온 문제에 대해 추리하여 알파벳을 입력함. 문자 전체를 입력해도됨.
계속 입력을 하고 결과를 받다가 서버에서 정답임이 확인되면 정답이라는 결과를 받는다.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int sock;
    char message[BUF_SIZE]; // 서버의 자식 프로세스로 부터 온 문자열
    int str_len;

    struct sockaddr_in serv_addr;

    if(argc!=3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }
    
    sock=socket(PF_INET, SOCK_STREAM, 0);   

    if(sock==-1)
        error_handling("socket() error");
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_addr.sin_port=htons(atoi(argv[2]));
    
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
        error_handling("connect() error!");
    
    str_len = read(sock, message, BUF_SIZE - 1); // 서버의 입력 값을 읽어와서
    message[str_len] = 0;
    fputs(message, stdout); // 클라이언트 화면에 출력함
    fputs("\n", stdout);
    fflush(stdout); // 표준출력버퍼에 있는 데이터를 모니터로 즉시 출력시켜준다.
        
    while(1) {
        // 내가 생각하는 알파벳 입력
        fflush(stdin);
        str_len = read(0, message, BUF_SIZE);
        message[str_len-1] = 0;

        write(sock, message, strlen(message)); // 서버의 자식 프로세스에 전송
        
        strcpy(message,"");
        
        str_len = read(sock, message, BUF_SIZE - 1); // 서버의 입력 값(자식 프로세스가 전송한 값)을 읽어와서
        message[str_len] = 0;

        fputs(message, stdout);
        fputs("\n", stdout);
        fflush(stdout);

        printf("==============================\n");
    }
    
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}