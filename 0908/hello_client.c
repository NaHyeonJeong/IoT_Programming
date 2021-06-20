#include<stdio.h> 
#include<stdlib.h> 
#include<string.h> 
#include<unistd.h> 
#include<arpa/inet.h> 
#include<sys/types.h> 
#include<sys/socket.h>

void error_handling(char *message);
void reverse(char *message, char *messageR);

int main(int argc, char **argv)
{
	int sock;
	struct sockaddr_in serv_addr;
	char message[30];
	char messageR[30] = ""; // reverse
	int str_len;
	
	if(argc!=3) {
		printf("Usage: %s <IP> <port> \n",argv[0]); 
		exit(1); 
	}
	
	sock=socket(PF_INET,SOCK_STREAM,0);
	if(sock==-1)
		error_handling("socket() error");
	
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	
	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1) 
		error_handling("connet() error");

	// server -> client (read)
	str_len=read(sock,message,sizeof(message)-1); 
	if(str_len==-1)
		error_handling("read() error!");
	message[str_len]=0;
	printf("Message from server : %s \n", message);
	
	// 1. reverse
	// 2. client -> server (wirte)
	reverse(message, messageR);
	write(sock, messageR, sizeof(messageR));
	
	close(sock);
	return 0;
}

void error_handling(char *message) {
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}

void reverse(char *message, char *messageR) {
	int i;
	int len = strlen(message);

	for (i = len; i > 0; i--) 
		messageR[len-i] = message[i-1];
}


