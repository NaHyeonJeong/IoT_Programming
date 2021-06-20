/*
프로그램 : IPC 서버 구축 - Guess Word - Server
작 성 자 : 나현정(201858042)
작 성 일 : 2020.10.20
설    명 : ipc를 이용한 행맨 게임 만들기...
서버와 클라이언트를 소켓으로 연경하고 서버의 부모 프로세스에서는 문제은행에서 한 문제를 랜덤하게 뽑고 자식 프로세스에 보냄.(알파벳 수도 보냄)
서버의 자식 프로세스는 부모 프로세스가 보낸 문제와 총 알파벳 수를 다시 클라이언트에 보냄.
클라이언트가 입력한 알파벳(자식 프로세스가 받아와서 부모 프로세스에 전달)을 문제와 비교해서 
있으면 찍고 없으면 이전에 만들어두었던 '_'이 있는 배열을 클라이언트에 다시 보냄.(자식 프로세스)
문자가 '_'이 있는 배열에서 최종적으로 완성이 되면(부모 프로세스) 정답이라는 것과 총 몇 회 시도했는지를 클라이언트에 알려줌(자식 프로세스)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include<time.h>

#define BUF_SIZE 100

void error_handling(char *message);
void z_handler(int sig);

int main(int argc, char *argv[])
{
    // 변수 선언
    int fd1[2], fd2[2]; // 파이프 관련
    char buffer[BUF_SIZE]; // 클라이언트가 입력한 알파벳(문자)를 저장하는 배열
    char buffer_len[BUF_SIZE];
    char buffer_rand[BUF_SIZE]; // 랜덤으로 추출한 문자가 저장되는 배열
    char buffer_rand_len[BUF_SIZE]; // 랜덤 추출 문자의 길이
    char token_len[10];
    char user_work[BUF_SIZE]=""; // 클라이언트가 입력한 것 중 정답이 있는 경우에만 밑줄이 포함된 내용을 저장할 배열
    char msg[BUF_SIZE];

    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;

    pid_t pid;
    struct sigaction act;
    int str_len, state, addr_size;
    
    char *word_bank[5] = {"banana", "apple", "android", "python", "airplane"};
    
    srand((unsigned)time(NULL)); // seed
    int isMatch = 0;

    // pipe, socket
    if(argc!=2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    if(pipe(fd1) < 0 || pipe(fd2) < 0) // 파이프 2개 생성 - 양방향 통신을 위함
        error_handling("Pipe() error!!");
    
    act.sa_handler = z_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;

    state = sigaction(SIGCHLD, &act, 0);
    if (state != 0) 
        error_handling("sigaction() error");

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    
    if(listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    puts("----- Game start -----");
    while(1) { // 서버는 무한루프, client가 계속 들어오면 계속 처리 가능
        addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &addr_size);
        if(clnt_sock == -1) continue;
        
        pid = fork(); // 자식 프로세스 생성
        if(pid == -1) { // 프로세스 생성 못함
           close(clnt_sock);
           continue;
        }
        else if(pid > 0) { //// 부모 프로세스
            puts("----- Connection Created -----");
            puts("Waiting for client...");
            close(clnt_sock); // client socket 닫음
            
            // 랜덤으로 문자열을 추출
            strcpy(buffer_rand, word_bank[rand() % 5]);
            printf("New question word =========> %s\n",buffer_rand); // 랜덤으로 추출한 문제를 서버에서 찍음
            
            memset(user_work,'\0',BUF_SIZE); // 초기화
            
            sprintf(buffer_rand_len, "%ld", strlen(buffer_rand)); // 정수형을 문자열 형태로 변환하는 방법
            // 자식 프로세스(클라이언트)에 문제를 보냄
            write(fd2[1], buffer_rand, BUF_SIZE); 
            // 자식 프로세스(클라이언트)에 추출한 문제가 총 몇개의 알파벳으로 구성된 단어인지를 알려줌
            write(fd2[1], strcat(buffer_rand_len, "개의 알파벳으로 구성된 단어를 맞추세요."), BUF_SIZE);

            for(int trynum=1;1==1;trynum++){
                // 자식 프로세스가 가져온 클라이언트의 알파벳을 읽어와서
                str_len = read(fd1[0], buffer, BUF_SIZE);
                if(strlen(buffer)>1){ // 클라이언트가 입력한게 알파벳 한 개가 아니라 여러개라면
                    if(strcmp(buffer_rand, buffer)==0){ // 완전히 맞춘 경우에만
                        puts("correct 01"); // 정답 인정
                        write(fd2[1], buffer, strlen(user_work));
                        break;
                    }
                    if(user_work[0]=='\0') { // 초기화 되어있는 부분이라면
                        for(int i = 0; i < strlen(buffer_rand); i++) user_work[i]='_';
                    }
                    write(fd2[1], user_work, strlen(user_work)); // 완성된 결과를 클라이언트에 전송
                }
                else{ // 클라이언트가 입력한게 알파벳 한 개라면
                    for(int j = 0; j < strlen(buffer); j++){
                        for(int i = 0; i < strlen(buffer_rand); i++){
                            if(buffer_rand[i]==buffer[j]) // 입력한 값이랑 문제에 있는 알파벳이랑 같으면
                                user_work[i] = buffer[j]; // _이 아닌 알파벳으로 교체
                            else if(user_work[i]=='\0')
                                user_work[i]='_';
                        }
                    }
                    write(fd2[1], user_work, strlen(user_work)); // 완성된 결과를 클라이언트에 전송
                }
            }
        }
        else { //// 자식 프로세스
            close(serv_sock); // 부모랑 클라이언트 연결 끊음
            
            str_len = read(fd2[0], buffer_rand, BUF_SIZE); // 부모 프로세스에서 온 문제를 읽음
            str_len = read(fd2[0], buffer_rand_len, BUF_SIZE); // 부모 프로세스에서 온 문제의 알파벳 수 읽음
            write(clnt_sock, buffer_rand_len, strlen(buffer_rand_len)); // 부모 프로세스에서 온 문제의 알파벳 수를 클라이언트에 보냄

            for(int trynum=1;1==1;trynum++){
                str_len=read(clnt_sock, buffer, BUF_SIZE); // 클라이언트가 입력한 알파벨을 읽어와서
                buffer[str_len] = 0;
                
                write(fd1[1], buffer, BUF_SIZE); // 부모 프로세스에게 전달해줌 pPos02
                memset(user_work,'\0',BUF_SIZE);
                str_len = read(fd2[0], user_work, 10); // 부모프로세스의 처리값을 읽음

                puts(user_work);
                puts(buffer_rand);
                
                strcat(msg,"확인결과=");
                strcat(msg,user_work);
                
                write(clnt_sock, msg, strlen(msg)); // client에 보냄-----------------------
                printf("sent =====>%s\n",msg);
                
                strcpy(msg,"");
                
                if(strcmp(user_work, buffer_rand) == 0){ // 문제와 계속해서 만들었던 user_work가 완전히 일치한다면
                    puts("correct 03");
                    puts("Disconnected..");
                    strcpy(msg,"\n[정답입니다....]");

                    char tnum[3];
                    sprintf(tnum,"%d",trynum);
                    strcat(msg,"\n[총 시도회수는 ");
                    strcat(msg,tnum);
                    strcat(msg," 입니다]");
                    
                    memset(user_work,'\0',BUF_SIZE);
                    write(clnt_sock, msg, strlen(msg)); // client에 보냄
                    close(clnt_sock);
                    break;
                }
            } // user_work가 buffer_rand와 완전히 같아지면 break;
            
            if(1==0){ ////////////
                puts("Disconnected..");
                close(clnt_sock);
                exit(0);
            }
        }
    }
    return 0;
}      

void z_handler(int sig)
{
    pid_t pid;
    int status;

    pid = waitpid(-1, &status, WNOHANG);
    printf("removed proc id: %d \n", pid);
    printf("Returned data : %d \n\n", WEXITSTATUS(status));
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}