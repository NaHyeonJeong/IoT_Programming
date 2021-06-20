/*  TCP FileEcho FTP  Client */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "practical.h"
#include "protocol.h"
/* opendir ��� */
#include <dirent.h>

/* �Լ� ���� �κ� */
void FileUploadProcess (int sock);
void FileDownloadProcess (int sock);
void EchoStringProcess (int sock);
void OpendirProcess(int sock);
void ExitProcess(int sock);
void SopendirProcess (int sock);

int main (int argc, char *argv[])
{
 /* �Է� �Ķ������ ����(argc)�� 3���� �˻� */
 if (argc != 3)
  {
   printf ("Usage : %s <Server Address>  <Server Port>\n", argv[0]);
   exit (1);
  }
 char *servIP = argv[1];
 in_port_t servPort = atoi(argv[2]);

 /* socket() : TCP ���� ���� */
 int sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
 if (sock == -1)
  error_handling ("socket() error");

 /* connect() : ���� ���� ��û */
 struct sockaddr_in serv_addr;
 memset (&serv_addr, 0, sizeof(serv_addr));
 serv_addr.sin_family = AF_INET;
 serv_addr.sin_addr.s_addr = inet_addr (servIP);
 serv_addr.sin_port = htons (servPort);
 if (connect (sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
  error_handling ("connect() error");

 /* �Է¹��� ��ɾ ���� �б� ó��  */
 char operation[BUFSIZE];
 memset(operation,0,sizeof(operation));
 printf(" welcome FTP & Echo Client !! \n");
 printf(" ftp command [ p)ut g)et l)s r)ls e)cho  exit)exit - >  ");
 fgets(operation,BUFSIZE,stdin);

 if (strcmp (operation, "p\n") == 0)
  FileUploadProcess (sock);
 else if (strcmp (operation, "g\n") == 0)
  FileDownloadProcess (sock);
 else if (strcmp (operation, "l\n") == 0)
  OpendirProcess (sock);
 else if (strcmp (operation, "e\n") == 0)
  EchoStringProcess (sock);
 else if (strcmp (operation, "r\n") == 0)
  SopendirProcess (sock);
 else if (strcmp (operation, "exit\n") == 0)
  ExitProcess(sock);
 else
  printf ("Usage : %s <Server Address>  <Server Port>\n", argv[0]);
  
 /* ���� ���� (close) */
 close (sock);
 return 0;
}

/* ����� �Լ� ���� �� */


void FileUploadProcess (int sock)
{
 /* msgType ���� : �ʵ�ũ��� 1����Ʈ��(uint8_t) ���� */
 uint8_t msgType = FileUpReq;
 
 ssize_t numBytesSent = send (sock, &msgType, sizeof (msgType), 0);
 if (numBytesSent == -1)
  error_handling ("send() error");
 else if (numBytesSent != sizeof (msgType))
  error_handling ("sent unexpected number of bytes");

 /* �����̸��� ������ ���� : �ʵ�ũ��� 256����Ʈ�� ���� */
 
 char fileName[FILENAMESIZE];
 memset (fileName, 0, FILENAMESIZE);
 printf(" fileName p)ut to server - >");
 scanf("%s",fileName);
 
 numBytesSent = send (sock, fileName, FILENAMESIZE, 0);
 if (numBytesSent == -1)
  error_handling ("send() error");
 else if (numBytesSent != FILENAMESIZE)
  error_handling ("sent unexpected number of bytes");

 /* ����ũ�⸦ ������ ���� : �ʵ�ũ��� 4����Ʈ(uint32_t)�� ���� */
 struct stat sb;
 if (stat (fileName, &sb) < 0)
  error_handling ("stat() error");
 uint32_t fileSize = sb.st_size;
 numBytesSent = send (sock, &fileSize, sizeof (fileSize), 0);
 if (numBytesSent == -1)
  error_handling ("send() error");
 else if (numBytesSent != sizeof (fileSize))
  error_handling ("sent unexpected number of bytes");

 /* ���ϳ����� ������ ���� : �ʵ�ũ��� ����(�� uint32_t ������ �� ũ�� */
 FILE *fp = fopen (fileName, "r");
 if (fp == NULL)
  error_handling ("fopen() error");
 printf("["); 
 while (!feof(fp))
 {
 
  char fileBuf[BUFSIZE];
  size_t numBytesRead = fread (fileBuf, sizeof (char), BUFSIZE, fp);
  if (ferror (fp))
   error_handling ("fread() error");
 
  numBytesSent = send (sock, fileBuf, numBytesRead, 0);
  if (numBytesSent == -1)
   error_handling ("send() error");
  else if (numBytesSent != numBytesRead)
   error_handling ("sent unexpected number of bytes");
  
  int i= sizeof(numBytesRead); // ���� ������
  int k= sizeof(fileSize); // �� ������ ������
  
  int m=(i*100)/k; // ���� ũ�Ⱑ �� �������� ���ۼ�Ʈ���� ���
  if(i<m)
  {
   
      
   printf("#");  
  }
  
 }
 printf("] 100 %% �Ϸ� \n");
 fclose (fp); /* ���� ���� �Ϸ� */

 /* �����κ��� ACK �޽��� ���� �� ȭ�鿡 ����/���п��θ� ��� */
 
 ssize_t FileAckRecv = recv(sock,&msgType,sizeof(msgType),MSG_WAITALL);
  if(FileAckRecv == -1)
   error_handling(" File recv error ");
 if(msgType == FileAck)
  printf(" ���������� %s ��(size:%d) ���ε� �Ͽ����ϴ� \n",fileName,fileSize);
 else
  printf(" ���ε� ����!! �ٽ� �õ����ּ��� \n");


}

void EchoStringProcess (int sock)
{

 /* msgType ���� ( ������ ���� �ڵ鷯 ȣ�� ) */
 uint8_t msgType = EchoReq;
        ssize_t numBytesSent = send (sock, &msgType, sizeof (msgType), 0);
        if (numBytesSent == -1)
                error_handling ("send() error");
        else if (numBytesSent != sizeof (msgType))
                error_handling ("sent unexpected number of bytes");
 
 printf("##### ftp> ���� ���񽺰� ���� �Ǿ����ϴ�  #####\n");
 printf(" ��  /quit �� �Է� �Ͻø� 1 : 1 ä���� ���� �˴ϴ�. ��\n");
 /* echoString ���� : �ʵ�ũ��� ������, ������ ���� socket/src/���̸�/04/echo_client.c ���� */
 while(1)
 {
 /* �Է� ���� ���� ����  */
 char message[BUFSIZE];
 memset(message,0,sizeof(message));

 /* ����ڷκ��� �Է� ���� */
 printf("ftp> : ");
 fgets(message,BUFSIZE,stdin);


 /* �Է� ���� ���� ���� */
 size_t echoStringLen = strlen(message);
 ssize_t numBytes = send (sock,message, echoStringLen, 0);
 if (numBytes == -1)
  error_handling ("send() error");
 
 /* �Է� ���� ���ڰ� /quit �� ��� �극��ũ */
 if(strcmp(message,"/quit\n") == 0)
  break;
 
 /* ���������κ��� ���ú�  */
 char buffer[BUFSIZE];
 memset(buffer,0,sizeof(buffer));
 numBytes = recv(sock,buffer,BUFSIZE,0);
 if(numBytes == -1 )
  error_handling(" recv() error ");
 
  if(strcmp(buffer,"/quit\n") == 0)
  { 
  printf(" ������ 1 :1 ä���� ���� �Ͽ����ϴ�.\n");
   break; 
  }
 printf("from  : %s ",buffer);
 }
 
 /* close() : ���� ���� */
 close(sock);
}
void FileDownloadProcess (int sock)
{
 /* ������ filedown �Լ� ȣ�� msg ���� */
 /* msgType ���� : �ʵ�ũ��� 1����Ʈ��(uint8_t) ���� */
 uint8_t msgType = DownReq;
 
 ssize_t numBytesSent = send (sock, &msgType, sizeof(msgType), 0);
 if (numBytesSent == -1)
  error_handling ("send() error");
 else if (numBytesSent != sizeof (msgType))
  error_handling(" number of bytes ");
 /* ����ڰ� �ٿ���� ���ϸ� ������ ���� */
 char fileName[FILENAMESIZE];
 memset(fileName,0,FILENAMESIZE);
 printf(" + + �ٿ� ���� ���ϸ��� �Է� �ϼ��� !! + + \n");
 printf(" get> ");
 scanf("%s",fileName);

 numBytesSent = send(sock,fileName,FILENAMESIZE,0);
 if (numBytesSent == -1)
                error_handling ("file name send() error");
 /* ���ϸ� ���� */
 strcat(fileName,"_recvd"); /* TODO Delete */
 /* ��û�� ������ ������ ����  */
 uint32_t filesize;
 ssize_t numBytesRcvd = recv(sock,&filesize,sizeof(filesize),MSG_WAITALL);
  if(numBytesRcvd == -1)
   error_handling(" recv error ");
  else if(numBytesRcvd == 0)
   error_handling(" peer connection closed  ");
  else if(numBytesRcvd != sizeof(filesize))
   error_handling(" recv unexpected number of bytes ");

 /* ��û�� ������ ������ ���� */
 
 FILE* fp = fopen(fileName,"w");
 if(fp == NULL)
  error_handling("fopen error");

 uint32_t rcvdFileSize = 0;
 printf("[ ");
 while (rcvdFileSize < filesize)
 {
  char fileBuf[BUFSIZE];
  numBytesRcvd = recv (sock, fileBuf, BUFSIZE, 0);
  if (numBytesRcvd == -1)
   error_handling ("recv() error");
  else if (numBytesRcvd == 0)
   error_handling ("peer connection closed");

  fwrite (fileBuf, sizeof (char), numBytesRcvd, fp);
  if (ferror (fp))
   error_handling ("fwrite() error");

  rcvdFileSize += numBytesRcvd;

  /* ���� ���� ��� */
  int i = sizeof(numBytesRcvd);
  int k = sizeof(filesize);
  int m=(i*100)/k;

  if(i<m)
  {
   printf("#");
   
  }

 }
 printf("] 100 %% �Ϸ� \n");
 fclose (fp); /* ���� ���� �Ϸ� */

 /* ���� �Ϸ� �޽��� ���� , ���ú� �Ϸ� �� �� ������ */
 if(rcvdFileSize ==filesize)
 {
  msgType=DownAck;
  ssize_t msgAckSent = send(sock,&msgType,sizeof(msgType),0);
   if(msgAckSent == -1)
    error_handling(" ack �޽��� ���� ���� ");
  printf(" %s ����(size:%d) �� �ٿ�ε� �Ͽ����ϴ�.\n",fileName,filesize);
 }
 else
 printf(" �ٿ�ε� ���� \n ");
}

void OpendirProcess (int sock)
{

 DIR *dp;
 struct dirent *d;

 /* ���� ���� */

 dp=opendir(".");
 if(dp == NULL)
  error_handling(" open dir() error");
 printf(" + + + + + + + + + + + + + + + + + + +\n"); 
 printf("#####  ����ǻ�� �����丮 �����Դϴ�. #####\n");
 /* ���� �б� */
 while((d=readdir(dp))!=NULL)
 {
  /* ���ʿ��� ���� ���� ��� */
  if(!strcmp(d->d_name,".") || !strcmp(d->d_name,"..") || !strcmp(d->d_name,".svn")) continue;
  fprintf(stdout,"%s\n",d->d_name);
 }
 printf(" + + + + + + + + + + + + + + + + + + +\n"); 
 printf("#####  ����� �Ϸ� �Ǿ����ϴ�. ###### \n");
 /* ��Ÿ ��ɾ� �߰� */
 char cmd[100];
 char buf[BUFSIZE];
 while(1)
 {
  fputs("\n",stdout);
  printf(" -----++��  ls Command Mode ��++------- \n");
  printf(".. [ cd  |  getcwd  |  ls  | exit ] ..\n");
  fputs("\n",stdout);
  printf("cmd> ");
  scanf("%s",cmd);
 
  /* getcwd : ���� �����丮 ��ġ ��� */
  if(strcmp(cmd,"getcwd") == 0 )
  {
   if(getcwd(buf,BUFSIZE)== NULL)
    error_handling(" getcwd error");
   fputs("\n",stdout);
   printf(" + ��  ���� ��ġ : %s\n",buf);
  }
  /* cd : �����丮 �̵� */
  if(strcmp(cmd,"cd") ==0)
  {
   printf("@@  �̵��� ��θ� �Է� �ϼ���.  @@ \n");
   scanf("%s",buf);
   if(chdir(buf)== -1)
    error_handling(" cd command error");
   printf("@@ %s ���� �̵� �Ͽ����ϴ�. @@ \n",buf);
  }
  /* ls : ���� �����丮 ���� ��� */
  if(strcmp(cmd,"ls") ==0)
  {
   printf(" + + + + + + + + + + + + + + + + + + +\n"); 
   dp=opendir(".");
   if(dp == NULL)
    error_handling(" open dir() error");
   while((d=readdir(dp))!=NULL)
   {
   if(!strcmp(d->d_name,".") || !strcmp(d->d_name,"..") || !strcmp(d->d_name,".svn")) continue;
   fprintf(stdout,"%s\n",d->d_name);
   }
   printf(" + + + + + + + + + + + + + + + + + + +\n"); 
   printf("#####  ����� �Ϸ� �Ǿ����ϴ�. ###### \n");
   fputs("\n",stdout);
  }
  if(strcmp(cmd,"exit")==0)
  {
  break;
  }
 }
 

 closedir(dp);

}

void SopendirProcess (int sock)
{
 
 
  /* �Ѱ����� rls ���μ��� ȣ�� �޽��� ����  */
  uint8_t msgType = DirReq;
  ssize_t numBytesSent = send (sock, &msgType, sizeof (msgType), 0);
  if (numBytesSent == -1)
   error_handling ("send() error");
  else if (numBytesSent != sizeof (msgType))
   error_handling ("sent unexpected number of bytes");
 
  /* ���� �λ縻 */

   printf("|-------------------------------------|\n");
   printf("|.........Welcome to Server ..........|\n");
   printf("|.....................................|\n");
   printf("|.........��  Command List �� ..........|\n");    
   printf("|.....................................|\n");
   printf("|[   rls  |  rcd   |  rcwd  |  exit  ]|\n");
   printf("|.....................................|\n");
   printf("|1.  rls : �����丮 ����Ʈ            |\n");
   printf("|2.  rcd : �����丮 �̵�              |\n");
   printf("|3.  rcwd : ���� ��ġ                 |\n");
   printf("|4.  exit : ����                      |\n");
   printf("..................��...................|\n");

  /* ����� �Է¿� ���� ��ɾ� ó�� */
  
 while(1)
 {
  char cmd[BUFSIZE];

   printf("     .' ��  rls Command List  �� '.  \n");    
   printf("[  rls  |  rcd   |  rcwd  |  exit  ] \n");
   fputs("\n",stdout);
   printf("cmd > ");
  fgets(cmd,BUFSIZE,stdin);
  /* ����� ���  ��û �۽� */
  ssize_t cmdsend = send(sock,cmd,BUFSIZE,0);
  if(cmdsend  == -1)
   error_handling(" user cmd send error ");

  /* ����� ��� ��û�� ���� ������ ó��  */
  if(strcmp(cmd,"rls\n")==0)
  {
   printf(" + + + + +  ���� �����丮�� ��� �մϴ�. + + + + + \n ");
   /* ���� �����丮 ���� ���� ���ú� */
   char recvbuf[BUFSIZE];
   memset(recvbuf,0,sizeof(recvbuf));
  
   ssize_t numBytesRcvd = recv(sock,recvbuf,BUFSIZE,0);
   if(numBytesRcvd == -1)
    error_handling("recv error");
   printf("%s",recvbuf);
   printf(" ++ ���� �����丮 ����� �Ϸ� �Ǿ����ϴ�. ++ \n");
   fputs("\n",stdout);
  }
  if(strcmp(cmd,"rcd\n")==0)
  { 
   /* �̵��� ��ġ �Է� ���� */
   char rcdbuf[BUFSIZE];
   memset(rcdbuf,0,sizeof(rcdbuf));
   printf(" ++ �̵��� ��ġ�� �Է� �ϼ���. ++ \n");
   printf(" ��ġ �ۼ� ��) /home/guest/\n");
   printf(" cmd > ");
   fgets(rcdbuf,BUFSIZE,stdin);
   /* ���๮�� (\n) ���� ���� : ���๮��������, ������ ����  */
   rcdbuf[strlen(rcdbuf)-1]='\0'; 
   /* �̵��� ��ġ ���� */
   ssize_t sendbuf = send(sock,rcdbuf,BUFSIZE,0);
   if(sendbuf == -1)
    error_handling(" send () error ");
   printf(" ++ %s �� �̵� �Ͽ����ϴ�. \n",rcdbuf);
   fputs("\n",stdout);
 
  }
  if(strcmp(cmd,"rcwd\n")==0)
  {
   printf("��  ���� ���� �����丮 ��ġ�� ��û �մϴ� �� \n");
   /* ���� ���� �����丮 ��ġ ���ú� */
   char  cwdbuf[BUFSIZE];
   ssize_t cwdsend = recv(sock,cwdbuf,BUFSIZE,0);
    if(cwdsend == -1)
     error_handling(" cwd send error");

   printf(" ���� ��ġ : %s \n",cwdbuf);
   fputs("\n",stdout);
  }

  /* ����� ��� exit ó�� */
  if(strcmp(cmd,"exit\n")==0)
  {
  ssize_t rcvdbuf = recv(sock,cmd,strlen(cmd),0);
  if(rcvdbuf == -1)
   error_handling(" exit recv error ");
  printf(" FTP �� ���� �˴ϴ�. \n");
  break;
  }
 }

}
// ���� ���μ���
void ExitProcess(int sock)
{
 exit(0);
 close(sock);
}



//��ó: https://disclosure.tistory.com/entry/Socket-Programin-in-C-2��°-Simple-FTP-server [HK Study Room :)]
