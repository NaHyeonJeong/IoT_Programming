/*
* 작 성 자 : 컴퓨터공학부 201858042 나현정
* 작 성 일 : 2020.11.02
* 프로그램 : anonymous ftp server
* 프로그램 설명 : 다수의 client가 get, put을 통해 파일을 download, upload하는 것을 처리함
get과 put의 기능을 할 수 있으며 파일 처리는 표준 입출력 파일 형태로 구현하여 소켓 프로그래밍에 사용되는 함수와 구별하였다.
client가 접속하거나 get, put을 하는 경우에 시간과 각 클라이언트의 port번호를 log_file.txt에 남기도록 하였다.
동일 파일이 존재하는 경우(250 error)에는 덮어 쓰는 것을 기본으로 하기 때문에 따로 처리를 하지 않았다.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include<fcntl.h>
#include<time.h>
#define FBUFSIZE 512
#define MBUFSIZE 100

void * clnt_connection(void *arg);
//void send_message(char * message, int len);
void error_handling(char *message);
void log_handling(char *message);

int clnt_number = 0;
int clnt_socks[10];
pthread_mutex_t mutx;

int main(int argc, char **argv) /* main start*/
{
    unsigned long int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    int clnt_addr_size;
    pthread_t thread;
    
    int str_len;
    char message[MBUFSIZE];
	char logtxt[300]="";

    if(argc!=2) {  
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    if(pthread_mutex_init(&mutx, NULL))
        error_handling("mutex init error");
    
    serv_sock = socket(PF_INET, SOCK_STREAM, 0); // 멀티캐스트 위한 TCP소켓 생성
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
        error_handling("bind() error");
    if(listen(serv_sock, 5) == -1)
        error_handling("listen() error");
    
	log_handling("Server Started..."); // server 시작!

    while(1){ 
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

        pthread_mutex_lock(&mutx); // critical section 사용 전에 잠그고 들어감   
        clnt_socks[clnt_number++] = clnt_sock; // critical section
        pthread_mutex_unlock(&mutx); // critical section 사용 후 열고 나옴
        pthread_create(&thread, NULL, clnt_connection, (void*)clnt_sock);
    }
    return 0;
} /* main end*/
void * clnt_connection(void *arg) /* 메시지 전송 쓰레드 실행 함수 */
{
    int clnt_sock = (unsigned long int)arg;
    int str_len = 0;
    char message[MBUFSIZE];
    char command[5], filename[20];
    int i;
    FILE *fp;
    size_t result;

	// server에 접속한 client 정보를 위한 변수들
    int client_len;
    struct sockaddr_in clientaddr;
    client_len = sizeof(clientaddr);
	char caddr[50]="";

	getpeername(clnt_sock, (struct sockaddr *)&clientaddr, &client_len);
	sprintf(caddr," (%s:%d) ", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	{ // log_file에 메시지 남기기 위함
		char logTxt[300]="";
		strcat(logTxt,caddr); strcat(logTxt,"연결");
		log_handling(logTxt);
	}

    while((str_len=read(clnt_sock, message, sizeof(message))) != 0) { /* out while start */
		message[str_len-1] = 0; // client가 입력한 내용의 마지막 부분에 \n이 들어간 것을 없애기 위함
		{ // log_file에 메시지 남기기 위함
			char logTxt[300]="";
			strcat(logTxt,caddr); strcat(logTxt,message);
			log_handling(logTxt);
		}
        sscanf(message, "%s", command); // client가 get, put 중 어떤 것을 입력했는지 알기 위함
		
		if(!strcmp(command, "get")) { /* get start */
			sscanf(message, "%s%s", filename, filename); // client가 보낸 파일 이름을 알기 위함

			struct stat sb; // 파일의 정보를 얻기 위한 stat 구조체
			if(stat(filename, &sb) >= 0) { // 파일 정보를 정상적으로 조회한 경우
				int sysErr = 0;
				ssize_t numBytesSent = send(clnt_sock, &sysErr, sizeof(sysErr), 0); // client에 현재 에러 없음을 알려줌
				
				// send() 실패
				if(numBytesSent == -1)
					error_handling("send() error");
				else if(numBytesSent != sizeof(sysErr)) 
					error_handling("byte 불일치");

				uint32_t fsize = sb.st_size; // 전송하고자 하는 파일의 크기를 가지는 변수
				numBytesSent = send(clnt_sock, &fsize, sizeof(fsize), 0); // client에 파일의 크기를 먼저 알려줌
				if(numBytesSent == -1) // client에 파일 크기 전송이 실패한 경우
					error_handling("send() error");
				else if(numBytesSent != sizeof(fsize)) // 전송하고자 하는 파일의 크기가 다른 경우
					error_handling("byte 불일치");

				fp = fopen(filename, "rb"); // 파일을 읽기모드로 열기
				if(fp == NULL)
					error_handling("fopen() error");

				while(!feof(fp)){
					char buffer[FBUFSIZE];
					result = fread(buffer, 1, FBUFSIZE, fp); // 전송하고자 하는 파일의 내용을 읽음
					if(ferror(fp))
						error_handling("fread() error");
					numBytesSent = send(clnt_sock, buffer, result, 0); // client에 파일의 내용을 보냄
					if(numBytesSent == -1) // client에 파일 내용 전송 실패의 경우 (300번 에러)
						error_handling("300");
					else if(numBytesSent != result)
						error_handling("byte 불일치");
				}
				fclose(fp); // 파일 닫기
				{ // log_file에 clinet 에게 파일 전송을 성공했음 메시지 남기기 위함
					char logTxt[300]="";
					strcat(logTxt,caddr); strcat(logTxt,message); strcat(logTxt," completed!");
					log_handling(logTxt);
				}
			}
			else { // 파일 정보를 정상적으로 조회하지 못한 경우
				int sysErr = 200; // 200번 에러
				ssize_t numBytesSent = send(clnt_sock, &sysErr, sizeof (sysErr), 0); // 200번 에러를 client에게 알려줌
				{ // log_file에 200번 에러를 메시지 남기기 위함
					char logTxt[300]="";
					strcat(logTxt,caddr); strcat(logTxt,message); strcat(logTxt," file not found!");
					log_handling(logTxt);
				}
			}
		} /* end of get */
		else if (!strcmp(command, "put")) { /* put start */
			sscanf(message, "%s%s", filename, filename); // client가 보낸 파일 이름을 알기 위함

 			int sysErr = 0;
			if(sysErr == 0) { // error가 발생하지 않은 경우
				int sysErr = 99;
				ssize_t numBytesSent = send(clnt_sock, &sysErr, sizeof(sysErr), 0); // 현재 에러가 없음을 client에 알려줌
				uint32_t fsize;
				ssize_t numBytesRcvd = recv(clnt_sock, &fsize, sizeof(fsize), 0); // client가 보내는 파일의 크기를 받음
				
				if(numBytesRcvd == -1)
					error_handling("recv() error ");
				else if(numBytesRcvd == 0)
					error_handling("peer connection closed ");
				else if(numBytesRcvd != sizeof(fsize))
					error_handling("recv() unexpected number of bytes ");
				
				if (!fsize) { // server에 client가 찾고있는 파일이 없다면
					printf("파일이 없습니다\n");
					continue;
				}

				FILE *fp = fopen(filename, "wb"); // 파일을 쓰기 모드로 열기
				if(fp == NULL)
					error_handling("fopen() error");

				uint32_t rcvdFileSize = 0; // client에서 받은 파일의 크기를 저장하기 위한 변수
				while(rcvdFileSize < fsize) { // 파일에 있는 내용 전송
					char fileBuf[FBUFSIZE];
					numBytesRcvd = recv(clnt_sock, fileBuf, FBUFSIZE, 0);
					
					if(numBytesRcvd == -1)
						error_handling("300"); // client로 부터 파일을 받지 못함 (300번 에러)
					else if(numBytesRcvd == 0)
						error_handling("peer connection closed");

					fwrite(fileBuf, sizeof(char), numBytesRcvd, fp); // 파일을 쓴다
					if(ferror(fp)) // 파일 쓰기 에러
						error_handling("fwrite() error");
					rcvdFileSize += numBytesRcvd;
				}
				fclose(fp); // 파일 닫기
				{ // log_file에 파일 업로드 성공 메시지 남기기 위함
					char logTxt[300]="";
					strcat(logTxt,caddr); strcat(logTxt,message); strcat(logTxt," completed!");
					log_handling(logTxt);
				}
			}
			else if(sysErr == 200)
				error_handling("200");
		} /* end of put */
    } /* out while end */
    pthread_mutex_lock(&mutx);
    for(i = 0; i < clnt_number; i++) { // client 연결 종료 시
        if(clnt_sock == clnt_socks[i]) {
            for( ; i < clnt_number - 1; i++)
                clnt_socks[i] = clnt_socks[i+1];
            break;
        }
    }
    clnt_number--; // client 수 감소
    pthread_mutex_unlock(&mutx);

    close(clnt_sock);
    return 0;
}
void error_handling(char *message) // error에 대한 처리 함수
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
void log_handling(char *message) // log_file 남기는 함수
{
	char logBuf[300] = "";
	time_t timeNow = time(NULL);
    struct tm t = *localtime(&timeNow);
    char acceptTime[200] = "";
    
	sprintf(logBuf,"%d-%d-%d %d:%d:%d %s", t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, message);
	FILE *fp = fopen("log_file.txt", "a"); // 파일 열기
	if(fp == NULL)
		error_handling("fopen() error");
	fputs(logBuf, fp); // 파일에 로그 남김
	fputs("\n", fp);
	fclose(fp); // 파일 닫음

    fputs(logBuf, stdout); // server 화면에 찍음
    fputc('\n', stdout);
}