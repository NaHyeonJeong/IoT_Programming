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
/* opendir 헤더 */
#include <dirent.h>

/* 함수 선언 부분 */
void FileUploadProcess (int sock);
void FileDownloadProcess (int sock);
void EchoStringProcess (int sock);
void OpendirProcess(int sock);
void ExitProcess(int sock);
void SopendirProcess (int sock);

int main (int argc, char *argv[])
{
 /* 입력 파라미터의 개수(argc)가 3인지 검사 */
 if (argc != 3)
  {
   printf ("Usage : %s <Server Address>  <Server Port>\n", argv[0]);
   exit (1);
  }
 char *servIP = argv[1];
 in_port_t servPort = atoi(argv[2]);

 /* socket() : TCP 소켓 생성 */
 int sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
 if (sock == -1)
  error_handling ("socket() error");

 /* connect() : 서버 연결 요청 */
 struct sockaddr_in serv_addr;
 memset (&serv_addr, 0, sizeof(serv_addr));
 serv_addr.sin_family = AF_INET;
 serv_addr.sin_addr.s_addr = inet_addr (servIP);
 serv_addr.sin_port = htons (servPort);
 if (connect (sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
  error_handling ("connect() error");

 /* 입력받은 명령어에 따라 분기 처리  */
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
  
 /* 세션 종료 (close) */
 close (sock);
 return 0;
}

/* 사용자 함수 선언 부 */


void FileUploadProcess (int sock)
{
 /* msgType 전송 : 필드크기는 1바이트로(uint8_t) 고정 */
 uint8_t msgType = FileUpReq;
 
 ssize_t numBytesSent = send (sock, &msgType, sizeof (msgType), 0);
 if (numBytesSent == -1)
  error_handling ("send() error");
 else if (numBytesSent != sizeof (msgType))
  error_handling ("sent unexpected number of bytes");

 /* 파일이름을 서버에 전송 : 필드크기는 256바이트로 고정 */
 
 char fileName[FILENAMESIZE];
 memset (fileName, 0, FILENAMESIZE);
 printf(" fileName p)ut to server - >");
 scanf("%s",fileName);
 
 numBytesSent = send (sock, fileName, FILENAMESIZE, 0);
 if (numBytesSent == -1)
  error_handling ("send() error");
 else if (numBytesSent != FILENAMESIZE)
  error_handling ("sent unexpected number of bytes");

 /* 파일크기를 서버에 전송 : 필드크기는 4바이트(uint32_t)로 고정 */
 struct stat sb;
 if (stat (fileName, &sb) < 0)
  error_handling ("stat() error");
 uint32_t fileSize = sb.st_size;
 numBytesSent = send (sock, &fileSize, sizeof (fileSize), 0);
 if (numBytesSent == -1)
  error_handling ("send() error");
 else if (numBytesSent != sizeof (fileSize))
  error_handling ("sent unexpected number of bytes");

 /* 파일내용을 서버에 전송 : 필드크기는 가변(위 uint32_t 변수의 값 크기 */
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
  
  int i= sizeof(numBytesRead); // 센드 사이즈
  int k= sizeof(fileSize); // 총 데이터 사이즈
  
  int m=(i*100)/k; // 보낸 크기가 총 데이터의 몇퍼센트인지 계산
  if(i<m)
  {
   
      
   printf("#");  
  }
  
 }
 printf("] 100 %% 완료 \n");
 fclose (fp); /* 파일 전송 완료 */

 /* 서버로부터 ACK 메시지 수신 후 화면에 성공/실패여부를 출력 */
 
 ssize_t FileAckRecv = recv(sock,&msgType,sizeof(msgType),MSG_WAITALL);
  if(FileAckRecv == -1)
   error_handling(" File recv error ");
 if(msgType == FileAck)
  printf(" 성공적으로 %s 을(size:%d) 업로드 하였습니다 \n",fileName,fileSize);
 else
  printf(" 업로드 실패!! 다시 시도해주세요 \n");


}

void EchoStringProcess (int sock)
{

 /* msgType 전송 ( 서버측 에코 핸들러 호출 ) */
 uint8_t msgType = EchoReq;
        ssize_t numBytesSent = send (sock, &msgType, sizeof (msgType), 0);
        if (numBytesSent == -1)
                error_handling ("send() error");
        else if (numBytesSent != sizeof (msgType))
                error_handling ("sent unexpected number of bytes");
 
 printf("##### ftp> 에코 서비스가 시작 되었습니다  #####\n");
 printf(" ☆  /quit 를 입력 하시면 1 : 1 채팅이 종료 됩니다. ☆\n");
 /* echoString 전송 : 필드크기는 가변적, 구현을 위해 socket/src/반이름/04/echo_client.c 참고 */
 while(1)
 {
 /* 입력 받을 버퍼 생성  */
 char message[BUFSIZE];
 memset(message,0,sizeof(message));

 /* 사용자로부터 입력 받음 */
 printf("ftp> : ");
 fgets(message,BUFSIZE,stdin);


 /* 입력 받은 문자 전송 */
 size_t echoStringLen = strlen(message);
 ssize_t numBytes = send (sock,message, echoStringLen, 0);
 if (numBytes == -1)
  error_handling ("send() error");
 
 /* 입력 받은 문자가 /quit 일 경우 브레이크 */
 if(strcmp(message,"/quit\n") == 0)
  break;
 
 /* 서버측으로부터 리시브  */
 char buffer[BUFSIZE];
 memset(buffer,0,sizeof(buffer));
 numBytes = recv(sock,buffer,BUFSIZE,0);
 if(numBytes == -1 )
  error_handling(" recv() error ");
 
  if(strcmp(buffer,"/quit\n") == 0)
  { 
  printf(" 서버가 1 :1 채팅을 종료 하였습니다.\n");
   break; 
  }
 printf("from  : %s ",buffer);
 }
 
 /* close() : 소켓 종료 */
 close(sock);
}
void FileDownloadProcess (int sock)
{
 /* 서버에 filedown 함수 호출 msg 전송 */
 /* msgType 전송 : 필드크기는 1바이트로(uint8_t) 고정 */
 uint8_t msgType = DownReq;
 
 ssize_t numBytesSent = send (sock, &msgType, sizeof(msgType), 0);
 if (numBytesSent == -1)
  error_handling ("send() error");
 else if (numBytesSent != sizeof (msgType))
  error_handling(" number of bytes ");
 /* 사용자가 다운받을 파일명 서버에 전송 */
 char fileName[FILENAMESIZE];
 memset(fileName,0,FILENAMESIZE);
 printf(" + + 다운 받을 파일명을 입력 하세요 !! + + \n");
 printf(" get> ");
 scanf("%s",fileName);

 numBytesSent = send(sock,fileName,FILENAMESIZE,0);
 if (numBytesSent == -1)
                error_handling ("file name send() error");
 /* 파일명 변경 */
 strcat(fileName,"_recvd"); /* TODO Delete */
 /* 요청한 파일의 사이즈 수신  */
 uint32_t filesize;
 ssize_t numBytesRcvd = recv(sock,&filesize,sizeof(filesize),MSG_WAITALL);
  if(numBytesRcvd == -1)
   error_handling(" recv error ");
  else if(numBytesRcvd == 0)
   error_handling(" peer connection closed  ");
  else if(numBytesRcvd != sizeof(filesize))
   error_handling(" recv unexpected number of bytes ");

 /* 요청한 파일의 내용을 수신 */
 
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

  /* 수신 상태 출력 */
  int i = sizeof(numBytesRcvd);
  int k = sizeof(filesize);
  int m=(i*100)/k;

  if(i<m)
  {
   printf("#");
   
  }

 }
 printf("] 100 %% 완료 \n");
 fclose (fp); /* 파일 수신 완료 */

 /* 수신 완료 메시지 전송 , 리시브 완료 된 후 전송함 */
 if(rcvdFileSize ==filesize)
 {
  msgType=DownAck;
  ssize_t msgAckSent = send(sock,&msgType,sizeof(msgType),0);
   if(msgAckSent == -1)
    error_handling(" ack 메시지 전송 실패 ");
  printf(" %s 파일(size:%d) 을 다운로드 하였습니다.\n",fileName,filesize);
 }
 else
 printf(" 다운로드 실패 \n ");
}

void OpendirProcess (int sock)
{

 DIR *dp;
 struct dirent *d;

 /* 파일 개방 */

 dp=opendir(".");
 if(dp == NULL)
  error_handling(" open dir() error");
 printf(" + + + + + + + + + + + + + + + + + + +\n"); 
 printf("#####  내컴퓨터 디텍토리 내용입니다. #####\n");
 /* 파일 읽기 */
 while((d=readdir(dp))!=NULL)
 {
  /* 불필요한 내용 빼고 출력 */
  if(!strcmp(d->d_name,".") || !strcmp(d->d_name,"..") || !strcmp(d->d_name,".svn")) continue;
  fprintf(stdout,"%s\n",d->d_name);
 }
 printf(" + + + + + + + + + + + + + + + + + + +\n"); 
 printf("#####  출력이 완료 되었습니다. ###### \n");
 /* 기타 명령어 추가 */
 char cmd[100];
 char buf[BUFSIZE];
 while(1)
 {
  fputs("\n",stdout);
  printf(" -----++☆  ls Command Mode ☆++------- \n");
  printf(".. [ cd  |  getcwd  |  ls  | exit ] ..\n");
  fputs("\n",stdout);
  printf("cmd> ");
  scanf("%s",cmd);
 
  /* getcwd : 현재 디텍토리 위치 출력 */
  if(strcmp(cmd,"getcwd") == 0 )
  {
   if(getcwd(buf,BUFSIZE)== NULL)
    error_handling(" getcwd error");
   fputs("\n",stdout);
   printf(" + ■  현재 위치 : %s\n",buf);
  }
  /* cd : 디텍토리 이동 */
  if(strcmp(cmd,"cd") ==0)
  {
   printf("@@  이동할 경로를 입력 하세요.  @@ \n");
   scanf("%s",buf);
   if(chdir(buf)== -1)
    error_handling(" cd command error");
   printf("@@ %s 으로 이동 하였습니다. @@ \n",buf);
  }
  /* ls : 현재 디텍토리 내용 출력 */
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
   printf("#####  출력이 완료 되었습니다. ###### \n");
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
 
 
  /* 총괄적인 rls 프로세서 호출 메시지 전송  */
  uint8_t msgType = DirReq;
  ssize_t numBytesSent = send (sock, &msgType, sizeof (msgType), 0);
  if (numBytesSent == -1)
   error_handling ("send() error");
  else if (numBytesSent != sizeof (msgType))
   error_handling ("sent unexpected number of bytes");
 
  /* 접속 인사말 */

   printf("|-------------------------------------|\n");
   printf("|.........Welcome to Server ..........|\n");
   printf("|.....................................|\n");
   printf("|.........♥  Command List ♥ ..........|\n");    
   printf("|.....................................|\n");
   printf("|[   rls  |  rcd   |  rcwd  |  exit  ]|\n");
   printf("|.....................................|\n");
   printf("|1.  rls : 디텍토리 리스트            |\n");
   printf("|2.  rcd : 디텍토리 이동              |\n");
   printf("|3.  rcwd : 현재 위치                 |\n");
   printf("|4.  exit : 종료                      |\n");
   printf("..................♡...................|\n");

  /* 사용자 입력에 따른 명령어 처리 */
  
 while(1)
 {
  char cmd[BUFSIZE];

   printf("     .' ★  rls Command List  ★ '.  \n");    
   printf("[  rls  |  rcd   |  rcwd  |  exit  ] \n");
   fputs("\n",stdout);
   printf("cmd > ");
  fgets(cmd,BUFSIZE,stdin);
  /* 사용자 명령  요청 송신 */
  ssize_t cmdsend = send(sock,cmd,BUFSIZE,0);
  if(cmdsend  == -1)
   error_handling(" user cmd send error ");

  /* 사용자 명령 요청에 따른 데이터 처리  */
  if(strcmp(cmd,"rls\n")==0)
  {
   printf(" + + + + +  서버 디텍토리를 출력 합니다. + + + + + \n ");
   /* 서버 디텍토리 버퍼 내용 리시브 */
   char recvbuf[BUFSIZE];
   memset(recvbuf,0,sizeof(recvbuf));
  
   ssize_t numBytesRcvd = recv(sock,recvbuf,BUFSIZE,0);
   if(numBytesRcvd == -1)
    error_handling("recv error");
   printf("%s",recvbuf);
   printf(" ++ 서버 디텍토리 출력이 완료 되었습니다. ++ \n");
   fputs("\n",stdout);
  }
  if(strcmp(cmd,"rcd\n")==0)
  { 
   /* 이동할 위치 입력 받음 */
   char rcdbuf[BUFSIZE];
   memset(rcdbuf,0,sizeof(rcdbuf));
   printf(" ++ 이동할 위치를 입력 하세요. ++ \n");
   printf(" 위치 작성 예) /home/guest/\n");
   printf(" cmd > ");
   fgets(rcdbuf,BUFSIZE,stdin);
   /* 개행문자 (\n) 제거 이유 : 개행문자있을시, 서버측 에러  */
   rcdbuf[strlen(rcdbuf)-1]='\0'; 
   /* 이동할 위치 전송 */
   ssize_t sendbuf = send(sock,rcdbuf,BUFSIZE,0);
   if(sendbuf == -1)
    error_handling(" send () error ");
   printf(" ++ %s 로 이동 하였습니다. \n",rcdbuf);
   fputs("\n",stdout);
 
  }
  if(strcmp(cmd,"rcwd\n")==0)
  {
   printf("☆  현재 서버 디텍토리 위치를 요청 합니다 ☆ \n");
   /* 현재 서버 디텍토리 위치 리시브 */
   char  cwdbuf[BUFSIZE];
   ssize_t cwdsend = recv(sock,cwdbuf,BUFSIZE,0);
    if(cwdsend == -1)
     error_handling(" cwd send error");

   printf(" 현재 위치 : %s \n",cwdbuf);
   fputs("\n",stdout);
  }

  /* 사용자 명령 exit 처리 */
  if(strcmp(cmd,"exit\n")==0)
  {
  ssize_t rcvdbuf = recv(sock,cmd,strlen(cmd),0);
  if(rcvdbuf == -1)
   error_handling(" exit recv error ");
  printf(" FTP 가 종료 됩니다. \n");
  break;
  }
 }

}
// 종료 프로세서
void ExitProcess(int sock)
{
 exit(0);
 close(sock);
}



//출처: https://disclosure.tistory.com/entry/Socket-Programin-in-C-2번째-Simple-FTP-server [HK Study Room :)]
