#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/file.h>
#include <netinet/in.h>

#define MAXLINE 1024
#define MAX_SOCK 512

char *escapechar = "exit\n";
int readline(int, char *, int);
int s;
struct Name // 클라이언트 닉네임
{
	char n[20];
	int len;
} name;

int main(int argc, char *argv[])
{
	char line[MAXLINE], recvline[MAXLINE], sendline[MAXLINE + 1]; // 채팅 메시지, 서버에서 받은거, 서버에게 보낼거
	int n, pid, size;
	struct sockaddr_in server_addr;
	int nfds;
	fd_set read_fds;

	if (argc < 4)
	{
		printf("실행방법 : %s 호스트 IP 주소 포트번호 사용자이름\n", argv[0]);
		return -1;
	}

	sprintf(name.n, "[%s]", argv[3]);
	name.len = strlen(name.n);
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Client : Can't open stream socket.\n");
		return -1;
	}

	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));

	// 연결요청
	if (connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		printf("Client : Can't connect to server.\n");
		return -1;
	}
	else
	{
		printf("접속에 성공했습니다..\n");
	}

	nfds = s + 1;
	FD_ZERO(&read_fds);
	while (1)
	{
		FD_SET(0, &read_fds);// 키보드로 입력한 것
		FD_SET(s, &read_fds);// 서버로 부터 온 것
		if (select(nfds, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0)
		{
			printf("select error\n");
			return -1;
		}
		// 서버로부터 수신한 메시지 처리
		if (FD_ISSET(s, &read_fds))
		{
			if ((size = recv(s, recvline, MAXLINE, 0)) > 0)
			{
				recvline[size] = '\0';
				printf("%s\n", recvline);
			}
			if (strcmp(recvline, "exit\0") == 0)// 서버에서 온 메시지가 종료 문자다
			{
				printf("서버에서 접속을 끊었습니다.\n");
				close(s);
				return -1;
			}
		} /* end of if_FD_ISSET */

		// 키보드 입력 처리
		if (FD_ISSET(0, &read_fds))
		{
			if (readline(0, sendline, MAXLINE) > 0)
			{
				size = strlen(sendline);
				sprintf(line, "%s %s", name.n, sendline);
				if (send(s, line, size + name.len, 0) != (size + name.len))
					printf("Error : Written error on socket.\n");
				if (size == 5 && strncmp(sendline, escapechar, 5) == 0)// exit + enter이기 때문에 5
				{
					printf("Good bye\n");
					close(s);
					return -1;
				}
			}
		} /* end of if_FD_ISSET */
	}/* end of while */
} /* end of main */

int readline(int fd, char *ptr, int maxlen)// 타이핑 하는 것
{
	int n, rc;
	char c;
	for (n = 1; n < maxlen; n++)
	{
		if ((rc = read(fd, &c, 1)) == 1)
		{
			*ptr++ = c;
			if (c == '\n')
				break;
		}
		else if (rc == 0)
		{
			if (n == 1)
				return (0);
			else
				break;
		}
	} /* end of for */
	*ptr = 0;
	return (n);
}
