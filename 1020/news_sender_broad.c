#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#define BUFSIZE 30
#define TRUE 1
#define FALSE 0
void error_handling(char *message);
int main(int argc, char **argv)
{
    int send_sock;
    struct sockaddr_in broad_addr;
    int state;
    FILE *fp;
    char buf[BUFSIZE];
    int so_broadcast=TRUE;
  
    if(argc!=3){
        printf("Usage : %s <Boradcast IP> <port>\n", argv[0]);
        exit(1);
    }
    send_sock=socket(PF_INET, SOCK_DGRAM, 0); /*브로드캐스트를 위해 UDP소켓 생성 */
    if(send_sock == -1)
        error_handling("socket() error");
 
  memset(&broad_addr, 0, sizeof(broad_addr));
  broad_addr.sin_family=AF_INET;
  broad_addr.sin_addr.s_addr=inet_addr(argv[1]);
  broad_addr.sin_port=htons(atoi(argv[2]));
  
  state=setsockopt(send_sock, SOL_SOCKET, SO_BROADCAST, (void*)&so_broadcast, sizeof(so_broadcast));
  if(state)
    error_handling("setsockopt() error");
  
  if((fp=fopen("news.txt", "r"))==NULL) /* news 파일 오픈 */
	  error_handling("fopen() error");
  while(!feof(fp)){	/* news 방송 */
	  fgets(buf, BUFSIZE, fp);
	  sendto(send_sock, buf, strlen(buf), 0, (struct sockaddr*)&broad_addr, sizeof(broad_addr));
	  sleep(2);
  }
  close(send_sock);
  return 0;
}
void error_handling(char *message)
{
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}

