#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>

#define BUFSIZE 30
void error_handling(char *message);
void z_handler(int sig);

int main(int argc, char **argv)
{
    int serv_sock;
    int clnt_sock;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;

    struct sigaction act;
    int addr_size, str_len, state;
    pid_t pid;
    char message[BUFSIZE];

    if(argc!=2){
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    act.sa_handler=z_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;

    state=sigaction(SIGCHLD, &act, 0);                              /* 시그널 핸들러 등록 */
    if(state != 0){
        puts("sigaction() error");
        exit(1);
    }
  
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);   
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
        error_handling("bind() error");

    if(listen(serv_sock, 5)==-1)
        error_handling("listen() error");


    while(1)
    {   
        addr_size=sizeof(clnt_addr);    
        clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr, &addr_size);
        if(clnt_sock==-1)
            continue;

    /* 클라이언트와의 연결을 독립적으로 생성 */
        if( (pid=fork()) ==-1) {                /* fork 실패 시 */
            close(clnt_sock);
            continue;
        }

        else if( pid>0 ) {                             /* 부모 프로세스인 경우 */
            puts("연결 생성");
            close(clnt_sock);
            continue;
        }
        else {                                       /* 자식 프로세스의 경우 */
            close(serv_sock);
            /* 자식 프로세스의 처리영역 : 데이터 수신 및 전송 */
     
            while( (str_len=read(clnt_sock, message, BUFSIZE)) != 0) {
		    write(clnt_sock, message, str_len);
		    write(1, message, str_len);
            }
            puts("연결 종료");
            close(clnt_sock);
            exit(0);
        }
    }
    return 0;
}

void z_handler(int sig)
{
    pid_t pid;
    int rtn;

    pid=waitpid(-1, &rtn, WNOHANG);
    printf("소멸된 좀비의 프로세스 ID : %d \n", pid);
    printf("리턴된 데이터 : %d \n\n", WEXITSTATUS(rtn));
}
void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

