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
    char buffer[BUF_SIZE];
    char intro[] = "Input(가위:0, 바위:1, 보:2) :  ";
    char win[] = "You Win!!\n";
    char lose[] = "You lose!!\n";
    char drawn[] = "You are drawn!!\n";

	int serv_sock, clnt_sock;
	struct sockaddr_in serv_addr, clnt_addr;
	
	pid_t pid;
	struct sigaction act;
	int str_len, state, addr_size;
	
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
		else if(pid > 0) { /* Parent Process : 가위바위보 결과 판정 역할 */
		    int result;
			puts("Connection Created..");  
			close(clnt_sock); // client socket 닫음
			
			write(1, intro, sizeof(intro)); // 서버 창에 가위바위보 선택지를 보임
			
			//read(0, buffer, BUF_SIZE); // 내가 작성한 내용을 읽음 (가위바위보)
			//read(fd1[0], &buffer[1], BUF_SIZE-1); // 클라이언트의 값을 읽어옴 (클라이언트의 값은 자식 프로세스가 가지고 있음, 가위바위보)
			buffer[0] = rand() % 3;
			read(fd1[0], &buffer[0], BUF_SIZE-1);
			// 부모 = 결과 판정의 역할
			result = who_win(buffer[0], buffer[1]);
			if (result == 0) { // 비김
				write(1, drawn, sizeof(drawn));
				write(fd2[1], drawn, sizeof(drawn));
			}
			else if(result == 1) { 
				write(1, win, sizeof(win)); // 부모가 이김  
				write(fd2[1], lose, sizeof(lose)); // 클라이언트가 짐
			}
			else { 
				write(1, lose, sizeof(lose)); // 부모가 짐
				write(fd2[1], win, sizeof(win)); // 클라이언트가 이김
			}
        }
		else { /* Child Process : 클라이언트랑 소통 창구 역할 */
			close(serv_sock); // 부모랑 클라이언트 연결 끊음
			
			write(clnt_sock, intro, sizeof(intro)); // 클라이언트에게 전달할 내용 (가위바위보 선택지)
			read(clnt_sock, buffer, BUF_SIZE); // 클라이언트 값을 읽어와서 
			
			write(fd1[1], buffer, 1); // 부모에게 알려줌 (fd1[1]을 통해 부모에게 전달)
			str_len = read(fd2[0], buffer, BUF_SIZE); // 부모에게서 온 값을 읽음 (fd2[0]을 통해 받음)
			write(clnt_sock, buffer, str_len); // 부모에게서 온 내용을 클라이언트에 보냄 (가위바위보 결과)

			puts("Disconnected..");
			close(clnt_sock);
			exit(0);
		}
	}
    return 0;
}      

int who_win(int a, int b) // 가위바위보
{
  int result;

  if (a == b) result = 0; // 비김
  else if (a % 3 == (b+1) % 3) result = 1; // a가 이김
  else result = -1; // a가 짐

  return result;
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
