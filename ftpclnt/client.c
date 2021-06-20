/*
* 작 성 자 : 컴퓨터공학부 201858042 나현정
* 작 성 일 : 2020.11.02
* 프로그램 : anonymous ftp client
* 프로그램 설명 : server에 get, put을 통해 파일을 download, upload하는 것을 처리함
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
#define FBUFSIZE 512
#define MBUFSIZE 100

void * send_message(void *arg);
void error_handling(char *message);

char message[MBUFSIZE];

int main(int argc, char **argv)
{
    unsigned long int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread; 
    void * thread_result;
    
    if(argc!=3){
        printf("Usage : %s <IP> <port>\n", argv[0]);   
        exit(1);
    }

    sock=socket(PF_INET, SOCK_STREAM, 0);
    if(sock==-1)
        error_handling("socket() error");
        
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_addr.sin_port=htons(atoi(argv[2])); 
    
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
        error_handling("100");
    
    printf("---- 입력 가능한 명령어 ----\n1) get filename\n2) put filename\n3) q(program end)\n");
    
    pthread_create(&snd_thread, NULL, send_message, (void*)sock);
    pthread_join(snd_thread, &thread_result);
    
    close(sock);
    return 0;
}
void * send_message(void *arg) /* 메시지 전송 쓰레드 실행 함수 */ 
{  
    int sock = (unsigned long int)arg;
    char command[5], filename[20];
	FILE *fp;
	size_t result;
	
    while(1) { /* start out while */
        fgets(message, MBUFSIZE, stdin); 	  
        if(!strcmp(message,"q\n")) {  /* 'q' 입력 시 종료 */          
            close(sock);          
            exit(0);
        }       
        
        sscanf(message, "%s", command);
		if (!strcmp(command, "get")) { /* get start */
            sscanf(message, "%s%s", filename, filename); // server에 파일 이름을 보내기 위함
			write(sock, message, strlen(message)); // server에 파일 이름 전송
            
			int sysErr = 9;
			ssize_t numBytesRcvd = recv(sock, &sysErr, sizeof(sysErr), 0); // server에서 error에 대한 정보 받음
			if(sysErr == 0) { // server에서 발생한 에러 없음
				uint32_t fsize;
				numBytesRcvd = recv(sock, &fsize, sizeof(fsize), 0); // server가 전송한 client가 요청한 파일의 크기를 받음
				
				// recv() 실패
				if(numBytesRcvd == -1) 
					error_handling("recv() error ");
				else if(numBytesRcvd == 0)
					error_handling("peer connection closed ");
				else if(numBytesRcvd != sizeof(fsize))
					error_handling("byte 불일치 ");
				if (!fsize) {
					printf("파일이 없습니다\n");
					continue;
				}

				FILE* fp = fopen(filename, "wb"); // 파일을 쓰기 모드로 열기
				if(fp == NULL)
					error_handling("fopen() error");

				uint32_t rcvdFileSize = 0;
				while (rcvdFileSize < fsize){
					char fileBuf[FBUFSIZE];
					numBytesRcvd = recv(sock, fileBuf, FBUFSIZE, 0);

					if (numBytesRcvd == -1) // server에서 파일 내용을 못 받아오는 경우 (300번 에러)
						error_handling("300");
					else if (numBytesRcvd == 0)
						error_handling("peer connection closed");

					fwrite(fileBuf, sizeof(char), numBytesRcvd, fp); // server에서 받아온 파일을 clinet 파일에씀
					if (ferror(fp))
						error_handling("fwrite() error");
					rcvdFileSize += numBytesRcvd;
				}
				fclose(fp); // 파일 닫기
				printf("다운로드 완료\n");
			}
			else if(sysErr == 200)
				error_handling("200");
        } /* enf of get */
		else if(!strcmp(command, "put")){ /* put start */
            sscanf(message, "%s%s", filename, filename); // server에 파일 이름을 보내기 위함
			
			struct stat sb; 
			if (stat(filename, &sb) >= 0) { // 파일 정보를 정상적으로 조회한 경우
				write(sock, message, strlen(message));

				int sysErr = 9;
				ssize_t numBytesRcvd = recv(sock, &sysErr, sizeof(sysErr), 0);
				if(sysErr!=99) 
					exit(1);

				uint32_t fsize = sb.st_size;
				ssize_t numBytesSent = send(sock, &fsize, sizeof (fsize), 0); 
				if (numBytesSent == -1) 
					error_handling("send() error"); 
				else if (numBytesSent != sizeof (fsize)) 
					error_handling("byte 불일치");

				fp = fopen(filename, "rb"); // 파일을 읽기 모드로 열기
				if(fp == NULL) {
					printf("fopen() error\n");
					exit(1);
				}

				while(!feof(fp)) {
					char buffer[FBUFSIZE];
					result = fread(buffer, 1, FBUFSIZE, fp);
					if (ferror(fp))
						error_handling("fread() error");
					numBytesSent = send(sock, buffer, result, 0);
					if (numBytesSent == -1)
						error_handling("send() error");
					else if (numBytesSent != result)
						error_handling("byte 불일치");
				}
				fclose(fp); // 파일 닫기
				printf("업로드 완료\n");
			}
			else // 파일 정보를 정상적으로 조회하지 못한 경우
				error_handling("200");
		} /* end of put */
    } /* end of out while */
}
void error_handling(char *message) // clinet에 에러에 대한 처리
{	
	// 에러가 발생하면 프로그램은 종료된다.
	// 단, 200번 에러는 파일이 없더라도 계속 돌아야 한다.
	char errMsg[100] = "";
	int preventExit = 0;
	
	if(strcmp(message,"100") == 0)
		strcpy(errMsg,"100 : Server 접속 에러");
	else if(strcmp(message,"200") == 0) {
		strcpy(errMsg,"200 : File Not-found");
		preventExit = 1;
	}
	else if(strcmp(message,"250")==0)
		strcpy(errMsg,"250 : File Existed(동일 파일 존재)");
	else if(strcmp(message,"300")==0)
		strcpy(errMsg,"300 : Transfer Failed");

	fputs(errMsg, stderr);
    fputc('\n', stderr);
	
	if(!preventExit)
		exit(1);
}
