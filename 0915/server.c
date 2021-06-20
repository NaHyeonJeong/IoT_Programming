#include<stdio.h> 
#include<stdlib.h> 
#include<string.h> 
#include<unistd.h> 
#include<arpa/inet.h> 
#include<sys/types.h> 
#include<sys/socket.h> 
#include<fcntl.h>
#include<time.h>

void error_handling(char *message);
void reverse(char *message, char *messageR);
void putStr(int file, char *str);

int main(int argc, char **argv)
{
    // Variables
	int serv_sock; 
	int clnt_sock;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr; // client ip...
	int clnt_addr_size;
	char message[100];
	char messageR[100];
	char *ipAdd;
	int str_len;
    int files;
    // time
    time_t timeNow = time(NULL);
    struct tm t = *localtime(&timeNow);
    char acceptTime[200] = "";
    double executeTime;
    char executeTimeStr[100] = "";
    clock_t start, end; // start time
    
	if(argc!=2) {
		printf("Usage: %s <port> \n",argv[0]);
		exit(1);
	}
	
	start = clock();
	serv_sock=socket(PF_INET,SOCK_STREAM,0); 
	if(serv_sock==-1) error_handling("socket() error");
	
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
    
	if(bind(serv_sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr))==-1) error_handling("bind() error");
	if(listen(serv_sock,5)==-1) error_handling("listen() error");
	
	clnt_addr_size=sizeof(clnt_addr);
	clnt_sock=accept(serv_sock,(struct sockaddr*)& clnt_addr,&clnt_addr_size);
	if(clnt_sock==-1)
		error_handling("accept() error");
	
	
	// client -> server
	// read client message
	str_len = read(clnt_sock, message, sizeof(message)-1); // serv_sock
	if(str_len == -1)
		error_handling("read() error!");
	message[str_len] = 0;
	printf("Message from client > %s \n", message);
	
	// read client ip address
	ipAdd = inet_ntoa(clnt_addr.sin_addr);
	printf("Client IP Address > %s \n", ipAdd);
	
	// 1. reverse
	// 2. server -> client(write)
	reverse(message, messageR);
	write(clnt_sock, messageR, sizeof(messageR));
	
	// open log file
	files = open("logFile.txt", O_RDWR | O_APPEND | O_CREAT, 0644);
	if(files == -1) error_handling("log file open() error!");
	
	// write <time> to log file
    sprintf(acceptTime, "Connect Time> %d-%d-%d %d:%d:%d", t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	putStr(files, acceptTime);
	// write <client ip address> to log file
	putStr(files, ipAdd);
   	// write <client message> to log file
   	putStr(files, message);
	// write <reverse message> to log file
	putStr(files, messageR);
	
	// close socket
	close(clnt_sock);

	// write <run time> to log file
   	end = clock();
	executeTime = (double)(end - start) / CLOCKS_PER_SEC;
	sprintf(executeTimeStr, "(Execute time is> %f)", executeTime);
	putStr(files, executeTimeStr);
	if(write(files, "\n", 1) == -1);
	
	// close log file
	close(files);
	
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

void putStr(int file, char *str) {
    if(write(file, str, strlen(str)) == -1)
        error_handling("write() error!");
    if(write(file, " | ", 3) == -1);
}

