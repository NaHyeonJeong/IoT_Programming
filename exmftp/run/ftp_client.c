
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

void FileUploadProcess (int sock, char *A_fileName);
void HandleFileDownload (int sock, char *A_filename);

int main (int argc, char* argv[])
{

 /* 명령 파라미터의 개수(argc)가 5인지를 검사 */
  if (argc != 3)
  {
  printf ("Usage : %s <Server Address> <Server Port>\n", argv[0]);
  exit (1);
  }
  
  char *servIP = argv[1];
  char operand[50];
  in_port_t servPort = atoi(argv[2]);

 /* socket() : 소켓생성 */
 int sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
 if (sock == -1)
 error_handling ("socket() error");
 
 /* connect() : 서버에 접속 */
 struct sockaddr_in serv_addr;
 memset (&serv_addr, 0, sizeof (serv_addr));
 serv_addr.sin_family = AF_INET;
 serv_addr.sin_addr.s_addr = inet_addr(servIP);
 serv_addr.sin_port = htons(servPort);
 if (connect(sock, &serv_addr, sizeof(serv_addr)) == -1)
 error_handling ("connect() error");
 
 int op;

 printf("Welcome to TJ's ftp\n");

 printf("ftp command [p)ut,  g)et  l)s   c)d   e)xit ] :  \n\n"); 
 
 while(1){
 printf("---------------------\n");
 printf("-       메 뉴       -\n");
 printf("-    1. upload      -\n");
 printf("-    2. download    -\n");
 printf("-    3. exit        -\n");
 printf("---------------------\n");

 printf("선택(숫자) : ");
 scanf("%d",&op);

 printf("----------------------\n\n");
 
 /* op에 따라 분기하여 처리 */  
 if (op == 1)
 { 
 printf("파일이름(확장자포함) : "); 
 scanf("%s",operand);
 FileUploadProcess (sock, operand); 
 }
 else if (op == 2)
 {
 printf("파일이름(확장자포함) : ");
 scanf("%s",operand);
 HandleFileDownload (sock, operand);
 }
 else if (op ==3)
  {
 
 /* msgType 전송 : 필드 크기는 1Bytes(uint8_t)로 고정 */

  uint8_t msgType = ExitReq;
  ssize_t numBytesSent = send (sock, &msgType, sizeof (msgType), 0);
  if (numBytesSent == -1)
  error_handling ("send() error");
  else if (numBytesSent != sizeof (msgType))
  error_handling ("sent unexpected number of bytes");
  break;
  }

  else
  printf ("없는 메뉴번호입니다. \n\n");

 } 

 /* close() : 소켓 종료 */

 close (sock); 
 return 0;
 }
 
 void FileUploadProcess (int sock, char *A_fileName)
 {

 /* msgType 전송 : 필드 크기는 1Bytes(uint8_t)로 고정 */
 uint8_t msgType = FileUpReq;
 ssize_t numBytesSent = send (sock, &msgType, sizeof (msgType), 0);
 if (numBytesSent == -1)
 error_handling ("send() error");
 else if (numBytesSent != sizeof (msgType))
 error_handling ("sent unexpected number of bytes");
 
 /* 파일이름을 서버에 전송 : 필드 크기를 256Bytes로 고정 */
 char fileName[FILENAMESIZE];
 memset (fileName, 0, FILENAMESIZE);
 strcpy (fileName, A_fileName);
 numBytesSent = send (sock, fileName, FILENAMESIZE, 0);
 if (numBytesSent == -1)
 error_handling ("send() error");
 else if (numBytesSent != FILENAMESIZE)
 error_handling ("sent unexpected number of bytes");
 
 /* 파일크기를 서버에 전송 : 필드 크기는 uint32_t로 고정 */
 struct stat sb;
 if (stat (fileName, &sb) < 0)
 error_handling ("stat() error");
 uint32_t fileSize = sb.st_size;
 uint32_t netFileSize = htonl (fileSize);
 numBytesSent = send (sock, &netFileSize, sizeof (netFileSize), 0);
 if (numBytesSent == -1)
 error_handling ("send() error");
 else if (numBytesSent != sizeof (netFileSize))
 error_handling ("sent unexpected number of bytes");
 
 /* 파일내용을 서버에 전송 : 필드 크기는 위 fileSize 변수값 */
 FILE *fp = fopen (fileName, "r");
 if (fp == NULL)
 error_handling ("fopen() error");
 
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
 }
 fclose (fp); /* 파일전송완료 */
 
 /* 서버로부터의 ACK메시지 수신 후 화면에 성공여부 출력 */
 ssize_t numBytesRcvd = recv (sock, &msgType, sizeof (msgType), MSG_WAITALL);
 if (numBytesRcvd == -1)
 error_handling ("recv() error");
 else if (numBytesRcvd == 0)
 error_handling ("peer connection closed");
 else if (numBytesRcvd != sizeof (msgType))
 error_handling ("recv unexpected number of bytes");
 
 if (msgType == FileAck)
 printf ("%s 업로드 성공!!(%u Bytes)\n", fileName, fileSize);
 else
 printf ("%s 파일 업로드 실패!!\n", fileName);
 }
 
 void HandleFileDownload (int sock,char *A_fileName)
 {
 /* msgType 전송 : 필드 크기는 1Bytes(uint8_t)로 고정 */
 uint8_t msgType = FileDownReq;
 ssize_t numBytesSent = send (sock, &msgType, sizeof (msgType), 0);
 if (numBytesSent == -1)
 error_handling ("send() error");
 else if (numBytesSent != sizeof (msgType))
 error_handling ("sent unexpected number of bytes");

 /* 파일이름을 서버에 전송 : 필드 크기를 256Bytes로 고정 */
 char fileName[FILENAMESIZE];
 memset (fileName, 0, FILENAMESIZE);
 strcpy (fileName, A_fileName);
 numBytesSent = send (sock, fileName, FILENAMESIZE,0);
 if (numBytesSent == -1)
 error_handling ("send() error");
 else if (numBytesSent != FILENAMESIZE)
 error_handling ("sent unexpected number of bytes");
 
 /* 파일 크기를 수신 : 필드 크기는 uint32_t로 고정 */
 strcat(fileName,"_down");
 printf("fileName= %s\n",fileName); 
 uint32_t netFileSize;
 uint32_t fileSize;
 ssize_t numBytesRcvd = recv (sock, &netFileSize, sizeof (netFileSize), MSG_WAITALL);
 if (numBytesRcvd == -1)
 error_handling ("recv() error");
 else if (numBytesRcvd == 0)
 error_handling ("peer connection closed");
 else if (numBytesRcvd != sizeof (netFileSize))
 error_handling ("recv unexpected number of bytes");
 fileSize = ntohl (netFileSize);
 

 /* 파일 내용을 수신 : 필드 크기는 위 uint32_t 내용 */
 FILE *fp = fopen (fileName, "w");
 if (fp == NULL)
 error_handling ("fopen() error");
 
 uint32_t rcvdFileSize = 0;
 while (rcvdFileSize < fileSize)
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
 
 }
 printf ("fileSize = %u\n다운로드 완료^^\n", fileSize);
 fclose (fp); /* 파일 수신 완료 */

 /* 파일수신 성공메시지(msgType:FileAck)를 서버에게 전송 */
 msgType = FileAck;
 numBytesSent = send (sock, &msgType, sizeof (msgType), 0);
 if (numBytesSent == -1)
 error_handling ("send() error");
 else if (numBytesSent != sizeof (msgType))
 error_handling ("sent unexpected number of bytes");
 }
