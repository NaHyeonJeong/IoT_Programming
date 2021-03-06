#include<stdio.h> 
#include<stdlib.h> 
#include<string.h> 
#include<unistd.h> 
#include<arpa/inet.h> 
#include<sys/types.h> 
#include<sys/socket.h> 
#include<fcntl.h>
#include<netinet/in.h>
#include<netdb.h>

void error_handling(char *message);

int main(int argc, char **argv)
{
    int sock;
	struct sockaddr_in serv_addr; // server ip...
	char query[200] = "";
    int str_len;
    int i;
    
   	struct hostent *myhost;
   	struct in_addr myinaddr; // dns -> 'ip'
 	struct sockaddr_in addr; // ip -> 'dns'
	
	if(argc!=3) {
		printf("Usage: %s <IP> <port> \n",argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET,SOCK_STREAM,0);
	if(sock==-1) error_handling("socket() error");
	
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]); // server ip address
	serv_addr.sin_port=htons(atoi(argv[2])); // port number
	
	// connect sDNS_server.c
	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1)  error_handling("connet() error");
	
	while(1) {
		fputs("Input Query (q to quit) >> ", stdout); // input ip or dns
		fgets(query, sizeof(query), stdin);
		query[strlen(query) - 1] = '\0';
		// write (query)
		write(sock, query, sizeof(query));
		// read : website ip or dns
		str_len = read(sock, query, sizeof(query)-1);
		if(str_len == -1) 
			error_handling("read() error!");
		query[str_len] = 0;
		printf("Query from server > %s \n", query);	

	}
	
	close(sock);
    return 0;
}

void error_handling(char *message) {
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}

