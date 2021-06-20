#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include<fcntl.h>
#include<signal.h>
#include<sys/file.h>
#include<netinet/in.h>
#include<sys/time.h>

#define MAXLINE 1024
#define MAX_SOCK 512 // ��ȭ �� �ִ� �ο� �� 
#define MAXPENDING 5                  

char *escapechar="exit\n";
char *escape="exit\0";

int CreateTCPServerSocket(unsigned short port); // ���� ���� �Լ�
void DieWithError(char *errorMessage);
int exitCheck(char *rline,char *escapechar,int len);

int main(int argc, char *argv[])
{
	char rline[MAXLINE], my_msg[MAXLINE],rline2[20];
	char *start="welcome to our chatting room";
	// ��� ������� �˷���
	char *comein="ä�ù濡 �Ѹ��� ������ ���Խ��ϴ�.\0"; 
	char *out="ä�ù� �̿��� �Ѹ��� �濡�� �������ϴ�.\n";
	int i,j,n,running=1,size_id; // running�� flag��
	int s, client_fd,clilen;
	int nfds; // ���ϵ�ũ���� ��
	fd_set read_fds;
	int num_chat = 0; // �� ���� ä���� �ϰ� �ִ°�
	int client_s[MAX_SOCK];
	struct sockaddr_in client_addr;
	struct timeval selTimeout; // �ð�

	if(argc<2) {
	    printf("usage:%s port number \n",argv[0]);
		return -1;		
    }
	printf("initializing chatroom server..\n");
	s = CreateTCPServerSocket(atoi(argv[1])); // ���� ���� ���� �Լ�
	
	nfds = s + 1;
	FD_ZERO(&read_fds); // read�� ���� �ʱ�ȭ
	printf("���� ���� �߽��ϴ�. �����Ϸ��� ���͸� ��������.\n");
	
	while(running) {
        if((num_chat-1) >= 0) 
            nfds = client_s[num_chat-1] + 1; // client_s : Ŭ���̾�Ʈ ��Ĺ ���
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(s,&read_fds);
        for(i=0;i<num_chat ;i++)
	        FD_SET(client_s[i], &read_fds); // � Ŭ���̾�Ʈ ä�� �޽����� ���´���
	    // �ð�
        selTimeout.tv_sec = 5;
        selTimeout.tv_usec = 0;
        
        //temps_fds = reads_fds;
        if(select(nfds, &read_fds, NULL, NULL,(struct timeval *)0) < 0) {
             printf("select error\n");
             return -1;
        }
				
        if(FD_ISSET(STDIN_FILENO,&read_fds)) {
            //�������� ������ �� ��� Ŭ���̾�Ʈ���� �������.
            printf("Shutting down server\n");
            getchar();
            for(j=0;j<num_chat;j++) { // ��� Ŭ���̾�Ʈ���� �������.
                send(client_s[j],escape,strlen(escape),0);
                shutdown(client_s[j],2); // Ŭ���̾�Ʈ�� �Դٰ��� �ϴ� ���� �������
            }
            running=0;
        } /* end of if */
					
        if(FD_ISSET(s, &read_fds)) {
            clilen = sizeof(client_addr);
            client_fd = accept(s,(struct sockaddr *)&client_addr,&clilen);
		
            if(client_fd != -1) { // select�� ��
                // ���ο� ������ ���°� ��� Ŭ���̾�Ʈ���� ��� 
                for(j=0;j<num_chat;j++) { 
                    send(client_s[j],comein,strlen(comein),0); 
                }
                client_s[num_chat] = client_fd; // �ο� �� ����
                num_chat++; // �ο� �� ����
					
                send(client_fd,start,strlen(start),0);
                printf(" %d ��° ������ �����߽��ϴ�.\n",num_chat); 
                //������ Ŭ���̾�Ʈ ID
                printf("%s\n",rline2);
            } /* end of inner if */
        } /* end of outer if */

        for(i=0;i<num_chat;i++) {
            if(FD_ISSET(client_s[i],&read_fds)) {     
                if((n=recv(client_s[i],rline,MAXLINE,0))>0) { // ������ ä���� ������ �о��
                    rline[n]='\0';
                    if(exitCheck(rline,escapechar,5) == 1) {  
                        shutdown(client_s[i],2);
                        printf("ä�ù� �̿��� �Ѹ��� �濡�� �������ϴ�..\n");
                        if(i!=num_chat-1)
                            client_s[i]=client_s[num_chat-1];
                        num_chat--;
                        for(j=0;j<num_chat;j++)
                            send(client_s[j],out,strlen(out),0);
                        continue; 
                    }
                    for(j=0;j<num_chat;j++)
                        send(client_s[j],rline,n,0); // Ŭ���̾�Ʈ���� �ϴ� ä�� �޽���
                } /* end of inner if */
            }  /* end of outer if */  
        } /* end of for */
    } /* end of while */
    return 0;
} /* end of main */	

int exitCheck(char *rline,char *escapechar,int len) // Ŭ���̾�Ʈ�� �����°�
{
    int i,max;
    char *tmp;
    max=strlen(rline);
    tmp=rline;
    for(i=0;i<max;i++)
    {
	    if(*tmp==escapechar[0])
	    {
	    if(strncmp(tmp,escapechar,len-1)==0)
	        return 1;
		
	    }
	    else {
	     tmp++;
	    }
	}
	return -1;
}

int CreateTCPServerSocket(unsigned short port)           
{  
    int sock;                                                     
    struct sockaddr_in echoServAddr;                         

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");                                         
                                                                
    memset(&echoServAddr, 0, sizeof(echoServAddr));        
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);      
    echoServAddr.sin_port = htons(port);                           
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

    if (listen(sock, MAXPENDING) < 0)                     
        DieWithError("listen() failed");
    return sock;
}

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}
