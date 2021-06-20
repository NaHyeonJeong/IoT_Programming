#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <time.h>

#define BUF_SIZE 100

void error_handling(char *message);
void z_handler(int sig);
int who_win(int a, int b);

int main(int argc, char *argv[])
{
    int fd1[2], fd2[2]; // 파이프 관련
    char buffer[BUF_SIZE]; // 0번에는 클라이언트가 입력한 문자 또는 문자열을, 1번에는 랜덤하게 뽑은 문자열을?

	int serv_sock, clnt_sock;
	struct sockaddr_in serv_addr, clnt_addr;
	
	pid_t pid;
	struct sigaction act;
	int str_len, state, addr_size;
	
	char *word_bank[5] = {"banana", "apple", "android", "python", "airplane"};
	srand((unsigned)time(NULL)); // seed

	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

    if(pipe(fd1) < 0 || pipe(fd2) < 0) // 파이프 2개 생성 - 양방향 통신을 위함
		error_handling("Pipe() error!!");
	
	act.sa_handler=z_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;

	state=sigaction(SIGCHLD, &act, 0);
	if (state != 0) 
		error_handling("sigaction() error");

	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
		error_handling("bind() error");
	
	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");
	
	while(1) { // 서버는 무한루프, client가 계속 들어오면 계속 처리 가능
		addr_size = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &addr_size);
		if(clnt_sock==-1)
			continue;
		
		pid=fork(); // 자식 프로세스 생성
		if(pid == -1) { // 프로세스 생성 못함
  		   close(clnt_sock);
		   continue;
		}
		/* 부모 프로세스 */
		else if(pid > 0) { 
			puts("Connection Created..");  
			close(clnt_sock); // client socket 닫음
			
			// 랜덤하게 게임에 사용할 문자열 추출
			strcpy(buffer, word_bank[rand() % 5]);
			puts(buffer); // 잘 뽑혔나 확인
			puts(&buffer[0]);
			puts(&buffer[1]);
        }
		/* 자식 프로세스 - 클라이언트와 부모 프로세스간의 소통 창구 역할 */
		else {
			close(serv_sock); // 부모랑 클라이언트 연결 끊음
			
			// read(clnt_sock, buffer, BUF_SIZE); // 클라이언트 값을 읽어와서 
			// write(fd1[1], buffer, 1); // 부모에게 알려줌 (fd1[1]을 통해 부모에게 전달)
			
			// str_len = read(fd2[0], buffer, BUF_SIZE); // 부모에게서 온 값을 읽음 (fd2[0]을 통해 받음)
			// write(clnt_sock, buffer, str_len); // 부모에게서 온 내용을 클라이언트에 보냄 (가위바위보 결과)

			puts("Disconnected..");
			close(clnt_sock);
			exit(0);
		}
	}
    return 0;
}      

void z_handler(int sig)
{
	pid_t pid;
	int status;

	pid=waitpid(-1, &status, WNOHANG);
	printf("removed proc id: %d \n", pid);
    printf("Returned data : %d \n\n", WEXITSTATUS(status));
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
