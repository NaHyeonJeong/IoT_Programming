
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "practical.h"
#include "protocol.h"

void HandleFileUpload (int clnt_sock);
void FileDownloadProcess (int clnt_sock);


int main (int argc, char *argv[])
{
 /* 명령 파라미터의 개수(argc)가 2인지 검사 */
  if (argc != 2)
  {
  printf ("Usage : %s <port>\n", argv[0]);    
  exit (1);    
  }
    
  in_port_t servPort = atoi(argv[1]);



 /* socket() : TCP 서버 소켓 생성 */
 int serv_sock = socket (PF_INET, SOCK_STREAM, 0);
 if (serv_sock == -1)
 error_handling ("socket() error");
 
 /* bind() : 서버가 사용할 포트/주소를 서버소켓과 묶음 */
 struct sockaddr_in serv_addr;
 memset (&serv_addr, 0, sizeof(serv_addr));
 serv_addr.sin_family = AF_INET;
 serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
 serv_addr.sin_port = htons (servPort);
 if (bind (serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
 error_handling ("bind() error");
 
 /* listen() : 서버 소켓을 리슨 소켓으로 변경 */
 if (listen (serv_sock, 5) == -1)
 error_handling ("bind() error");
 
 /* accept() : 연결 후 생성된 클라이언트소켓을 리턴 */
 struct sockaddr_in clnt_addr;
 socklen_t clnt_addr_len = sizeof (clnt_addr);
 int clnt_sock = accept (serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);
 if (clnt_sock == -1)
 error_handling ("accept() error");
 
 /* 접속한 클라이언트의 정보를 화면에 출력 */
 printf ("Connected from %s:%d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port) );
 
 while(1){
 /* msgType 수신 : 필드 크기는 1Bytes(uint8_t)로 고정 */
 uint8_t msgType;
 ssize_t numBytesRcvd = recv (clnt_sock, &msgType, sizeof (msgType), MSG_WAITALL);
 if (numBytesRcvd == -1)
 error_handling ("recv() error");
 else if (numBytesRcvd == 0)
 error_handling ("peer connection closed");
 else if (numBytesRcvd != sizeof (msgType))
 error_handling ("recv unexpected number of bytes");
 
 /* msgType 값에 따라 분기 처리 */
 if (msgType == FileUpReq)
 HandleFileUpload (clnt_sock); 
 
 /* 파일 업로드 처리 */
 else if(msgType == FileDownReq)
 FileDownloadProcess (clnt_sock); 
 
 /*파일 다운로드 처리 */
 else if (msgType == ExitReq)

 break;

 else
 printf("error_not define msg");
 }

 /* close() : 소켓을 닫음 */
 close (clnt_sock);
 close (serv_sock);
 return 0;
 }
 

 void HandleFileUpload (int clnt_sock) 
 { 

 /* 파일 이름을 수신 : 필드 크기는 256Bytes로 고정 */ 
 char fileName[FILENAMESIZE]; 
 ssize_t numBytesRcvd = recv (clnt_sock, fileName, FILENAMESIZE, MSG_WAITALL); 
 if (numBytesRcvd == -1) 
 error_handling ("recv() error"); 
 else if (numBytesRcvd == 0) 
 error_handling ("peer connection closed"); 
 else if (numBytesRcvd != FILENAMESIZE) 
 error_handling ("recv unexpected number of bytes"); 
 strcat (fileName, "_up"); 
 printf ("fileName = %s\n", fileName); 
  
 /* 파일 크기를 수신 : 필드 크기는 uint32_t로 고정 */ 
 uint32_t netFileSize; 
 uint32_t fileSize; 
 numBytesRcvd = recv (clnt_sock, &netFileSize, sizeof (netFileSize), MSG_WAITALL); 
 if (numBytesRcvd == -1) 
 error_handling ("recv() error"); 
 else if (numBytesRcvd == 0) 
 error_handling ("peer connection closed"); 
 else if (numBytesRcvd != sizeof (netFileSize)) 
 error_handling ("recv unexpected number of bytes"); 
 fileSize = ntohl (netFileSize); 
 printf ("fileSize = %u\n", fileSize); 
  
 /* 파일 내용을 수신 : 필드 크기는 위 uint32_t 내용 */ 
 FILE *fp = fopen (fileName, "w"); 
 if (fp == NULL) 
 error_handling ("fopen() error"); 
  
 uint32_t rcvdFileSize = 0; 
 while (rcvdFileSize < fileSize)
 { 
 char fileBuf[BUFSIZE]; 
 numBytesRcvd = recv (clnt_sock, fileBuf, BUFSIZE, 0); 
 if (numBytesRcvd == -1) 
 error_handling ("recv() error"); 
 else if (numBytesRcvd == 0) 
 error_handling ("peer connection closed"); 
  
 fwrite (fileBuf, sizeof (char), numBytesRcvd, fp); 
 if (ferror (fp)) 
 error_handling ("fwrite() error"); 
  
 rcvdFileSize += numBytesRcvd; 
 } 
 fclose (fp); 
 
 /* 파일 수신 완료 */ 
  
 /* 파일수신 성공메시지(msgType:FileAck)를 클라이언트에게 전송 */ 
 uint8_t msgType = FileAck; 
 ssize_t numBytesSent = send (clnt_sock, &msgType, sizeof (msgType), 0); 
 if (numBytesSent == -1) 
 error_handling ("send() error"); 
 else if (numBytesSent != sizeof (msgType)) 
 error_handling ("sent unexpected number of bytes"); 
 } 
  
 void FileDownloadProcess (int clnt_sock) 
 { 
  
 /* 파일 이름을 수신 : 필드 크기는 256Bytes로 고정 */ 
 char fileName[FILENAMESIZE]; 
 ssize_t numBytesRcvd = recv (clnt_sock, fileName, FILENAMESIZE, MSG_WAITALL); 
 if (numBytesRcvd == -1) 
 error_handling ("recv() error"); 
 else if (numBytesRcvd == 0) 
 error_handling ("peer connection closed"); 
 else if (numBytesRcvd != FILENAMESIZE) 
 error_handling ("recv unexpected number of bytes"); 
  
 /* 파일크기를 클라에 전송 : 필드 크기는 uint32_t로 고정 */ 
 struct stat sb; 
 if (stat (fileName, &sb) < 0) 
 error_handling ("stat() error"); 
 uint32_t fileSize = sb.st_size; 
 uint32_t netFileSize = htonl (fileSize); 
 ssize_t numBytesSent = send (clnt_sock, &netFileSize, sizeof (netFileSize), 0); 
 if (numBytesSent == -1) 
 error_handling ("send() error"); 
 else if (numBytesSent != sizeof (netFileSize)) 
 error_handling ("sent unexpected number of bytes"); 
  
 /* 파일내용을 클라에 전송 : 필드 크기는 위 fileSize 변수값 */ 
 FILE *fp = fopen (fileName, "r"); 
 if (fp == NULL) 
 error_handling ("fopen() error"); 
  
 while (!feof(fp)) 
 { 
 char fileBuf[BUFSIZE]; 
 size_t numBytesRead = fread (fileBuf, sizeof (char), BUFSIZE, fp); 
 if (ferror (fp)) 
 error_handling ("fread() error"); 
  
 numBytesSent = send (clnt_sock, fileBuf, numBytesRead, 0); 
 if (numBytesSent == -1) 
 error_handling ("send() error"); 
 else if (numBytesSent != numBytesRead) 
 error_handling ("sent unexpected number of bytes"); 
 } 
 fclose (fp); 
 
 /* 파일전송완료 */ 
 
 /* 클라이언트로부터의 ACK메시지 수신 후 화면에 성공여부 출력 */
 
 uint8_t msgType; 
 numBytesRcvd = recv (clnt_sock, &msgType, sizeof (msgType), MSG_WAITALL); 
 if (numBytesRcvd == -1) 
  error_handling ("recv() error");
  else if (numBytesRcvd == 0)
  error_handling ("peer connection closed");
  else if (numBytesRcvd != sizeof (msgType))
  error_handling ("recv unexpected number of bytes");
   
  if (msgType == FileAck)
  printf ("%s 파일 전송 성공!!(%u Bytes)\n", fileName, fileSize);
  else
  printf ("%s 파일 전송 실패!!\n", fileName);
  }