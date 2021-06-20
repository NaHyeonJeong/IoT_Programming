#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#define BUFSIZE 30
void error_handling(char *message);
int main(int argc, char **argv)
{
  int recv_sock;
  struct sockaddr_in addr;
  int str_len;
  char buf[BUFSIZE];
  if(argc!=2){
    printf("Usage : %s  <port>\n", argv[0]);
    exit(1);
  }
  
  recv_sock=socket(PF_INET, SOCK_DGRAM, 0);    /*브로드캐스트를 위한 UDP소켓 생성 */
if(recv_sock == -1)
    error_handling("socket() error");
 memset(&addr, 0, sizeof(addr));
 addr.sin_family=AF_INET;
 addr.sin_addr.s_addr=htonl(INADDR_ANY);	
 addr.sin_port=htons(atoi(argv[1]));	   /* 브로드캐스트 port 설정 */
 if(bind(recv_sock, (struct sockaddr*)&addr, sizeof(addr))==-1)
	  error_handling("bind() error");
  
  

while(1){
	  str_len=recvfrom(recv_sock, buf, BUFSIZE-1, 0, NULL, 0);
	  if(str_len<0) break;
	  buf[str_len]=0;
	  fputs(buf, stdout);
  }
  close(recv_sock);
  return 0;
}
void error_handling(char *message)
{
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
