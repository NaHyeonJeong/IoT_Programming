#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
	char message[BUF_SIZE];
	int str_len;

	struct sockaddr_in serv_addr;

	if(argc!=3) {
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
		error_handling("connect() error!");
	
	// str_len = read(sock, message, BUF_SIZE - 1); // 서버의 입력 값을 읽어와서
    // message[str_len] = 0;
    // fputs(message, stdout); // 클라이언트 화면에 출력함 (가위바위보 선택지 읽어옴)
    // fflush(stdout);
	
	// str_len = read(0, message, BUF_SIZE);
	// write(sock, message, str_len);
	
	// str_len = read(sock, message, BUF_SIZE - 1);
	// message[str_len] = 0;
	// fputs(message, stdout);
	
    close(sock);
    return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
