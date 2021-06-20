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
	struct hostent *host;
	struct sockaddr_in addr;
	int i;
	
	if(argc!=2){
		printf("Usage : %s <IP>\n", argv[0]);
		exit(1);
	}
	
    memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr=inet_addr(argv[1]);
	
	host=gethostbyaddr((char*)&addr.sin_addr, 4, AF_INET);
	
	if(!host) error_handling("gethost... error");
	
	printf("Officially name : %s \n\n", host->h_name);
	
	puts("Aliases-----------");
	for(i=0;host->h_aliases[i]; i++){
		puts(host->h_aliases[i]);
	}
	
	

    printf("Address Type : %s \n", host->h_addrtype==AF_INET? "AF_INET":"AF_INET6");  
	puts("IP Address--------");
	for(i=0; host->h_addr_list[i]; i++){
		puts( inet_ntoa( *(struct in_addr*)host->h_addr_list[i] ));
	}
	
	return 0;
}

void error_handling(char *message) { 
	fputs(message,stderr); 
	fputc('\n',stderr);
	exit(1);
}

