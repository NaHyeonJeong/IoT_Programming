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
	struct sockaddr_in serv_addr; // server ip...
	char message[100]="";
	int str_len;
	char *sipAdd;
	
	if(argc!=3) {
		printf("Usage: %s <IP> <port> \n",argv[0]);
		exit(1);
	}
    
	printf("Input Message > ");
	scanf("%s", message);
	
	sock=socket(PF_INET,SOCK_STREAM,0);
	if(sock==-1) error_handling("socket() error");
	
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]); // server ip address
	serv_addr.sin_port=htons(atoi(argv[2])); // port number
	
	// connect
	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1)  error_handling("connet() error");

    // client -> server (wirte)
	write(sock, message, sizeof(message));
	
	// server -> client (read)
	str_len=read(sock, message, sizeof(message)-1); 
	if(str_len==-1) error_handling("read() error!");
	message[str_len]=0;
	printf("Message from server > %s \n", message);
	
	// read server ip address
	sipAdd = inet_ntoa(serv_addr.sin_addr);
	printf("Server IP Address > %s \n", sipAdd);
	
	close(sock);
	return 0;
}

void error_handling(char *message) {
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}


