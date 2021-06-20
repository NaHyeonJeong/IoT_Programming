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
    int serv_sock;
	int clnt_sock;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;
	char message[BUFSIZ];
	int str_len;
		
	if(argc!=2)
	{
	printf("Usage: %s <port> \n",argv[0]);
	exit(1);
	}

	serv_sock=socket(PF_INET,SOCK_STREAM,0);
	if(serv_sock==-1)
	error_handling("socket() error");
    
    memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr))==-1)
	error_handling("bind() error");

    if(listen(serv_sock,5)==-1)
        error_handling("listen() error");

	// 무한루프
    for( ; ; ) {
		// 이 무한루프에는 데이터 수신은 없고 데이터 전송만 있음	
        clnt_addr_size=sizeof(clnt_addr);
        clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);    
        if(clnt_sock==-1) {      
	        error_handling("accept() error");
	        break;
        }
        
        /* 클라이언트에 데이터 전송 후 종결 */
        // 밖에 있던 while문이 안으로 들어와도 상관이 없더라!
		while((str_len=read(clnt_sock,message,BUFSIZ))!=0) {
			//write(clnt_sock, message, sizeof(message));
			write(clnt_sock, message, str_len);
		}
        
        close(clnt_sock);  
    }

    //데이터 수신 및 전송 //
    // while((str_len=read(clnt_sock,message,BUFSIZ))!=0) { // 데이터 수신
    // 	write(clnt_sock, message, str_len); // 데이터 전송
    // 	write(1,message,str_len); // 없어도 상관은 없음
    // }
    
    //close(clnt_sock);
    return 0;
}

void error_handling(char *message) { 
	fputs(message,stderr); 
	fputc('\n',stderr);
	exit(1);
}
