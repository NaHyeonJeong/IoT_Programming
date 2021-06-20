#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#define TTL 64
#define BUFSIZE 30
void error_handling(char *message);

int main(int argc, char **argv)
{
    int send_sock;
    struct sockaddr_in multi_addr;
    int multi_TTL=TTL;
    int state;
    FILE *fp;
    char buf[BUFSIZE];
    
    if(argc!=3){
        printf("Usage : %s <GrouoIP> <port>\n", argv[0]);
        exit(1);
    }  
    send_sock=socket(PF_INET, SOCK_DGRAM, 0); /*멀티캐스트를 위한 UDP소켓 생성 */
  
    if(send_sock == -1)
        error_handling("socket() error");
        
    memset(&multi_addr, 0, sizeof(multi_addr));
    multi_addr.sin_family=AF_INET;
    multi_addr.sin_addr.s_addr=inet_addr(argv[1]); /* 멀티캐스트 IP 설정 */
    multi_addr.sin_port=htons(atoi(argv[2]));		/* 멀티캐스트 port 설정 */
    state=setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &multi_TTL, sizeof(multi_TTL));
    if(state)
        error_handling("setsockopt() error");
  
    if((fp=fopen("news.txt", "r"))==NULL) /* news 파일 오픈 */
	    error_handling("fopen() error");
    while(!feof(fp)){	/* news 방송 */
	    fgets(buf, BUFSIZE, fp);
	    sendto(send_sock, buf, strlen(buf), 0, (struct sockaddr*)&multi_addr, sizeof(multi_addr));
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
