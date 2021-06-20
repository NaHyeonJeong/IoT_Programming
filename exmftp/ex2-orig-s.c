
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
 /* ��� �Ķ������ ����(argc)�� 2���� �˻� */
  if (argc != 2)
  {
  printf ("Usage : %s <port>\n", argv[0]);    
  exit (1);    
  }
    
  in_port_t servPort = atoi(argv[1]);



 /* socket() : TCP ���� ���� ���� */
 int serv_sock = socket (PF_INET, SOCK_STREAM, 0);
 if (serv_sock == -1)
 error_handling ("socket() error");
 
 /* bind() : ������ ����� ��Ʈ/�ּҸ� �������ϰ� ���� */
 struct sockaddr_in serv_addr;
 memset (&serv_addr, 0, sizeof(serv_addr));
 serv_addr.sin_family = AF_INET;
 serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
 serv_addr.sin_port = htons (servPort);
 if (bind (serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
 error_handling ("bind() error");
 
 /* listen() : ���� ������ ���� �������� ���� */
 if (listen (serv_sock, 5) == -1)
 error_handling ("bind() error");
 
 /* accept() : ���� �� ������ Ŭ���̾�Ʈ������ ���� */
 struct sockaddr_in clnt_addr;
 socklen_t clnt_addr_len = sizeof (clnt_addr);
 int clnt_sock = accept (serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);
 if (clnt_sock == -1)
 error_handling ("accept() error");
 
 /* ������ Ŭ���̾�Ʈ�� ������ ȭ�鿡 ��� */
 printf ("Connected from %s:%d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port) );
 
 while(1){
 /* msgType ���� : �ʵ� ũ��� 1Bytes(uint8_t)�� ���� */
 uint8_t msgType;
 ssize_t numBytesRcvd = recv (clnt_sock, &msgType, sizeof (msgType), MSG_WAITALL);
 if (numBytesRcvd == -1)
 error_handling ("recv() error");
 else if (numBytesRcvd == 0)
 error_handling ("peer connection closed");
 else if (numBytesRcvd != sizeof (msgType))
 error_handling ("recv unexpected number of bytes");
 
 /* msgType ���� ���� �б� ó�� */
 if (msgType == FileUpReq)
 HandleFileUpload (clnt_sock); 
 
 /* ���� ���ε� ó�� */
 else if(msgType == FileDownReq)
 FileDownloadProcess (clnt_sock); 
 
 /*���� �ٿ�ε� ó�� */
 else if (msgType == ExitReq)

 break;

 else
 printf("error_not define msg");
 }

 /* close() : ������ ���� */
 close (clnt_sock);
 close (serv_sock);
 return 0;
 }
 

 void HandleFileUpload (int clnt_sock) 
 { 

 /* ���� �̸��� ���� : �ʵ� ũ��� 256Bytes�� ���� */ 
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
  
 /* ���� ũ�⸦ ���� : �ʵ� ũ��� uint32_t�� ���� */ 
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
  
 /* ���� ������ ���� : �ʵ� ũ��� �� uint32_t ���� */ 
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
 
 /* ���� ���� �Ϸ� */ 
  
 /* ���ϼ��� �����޽���(msgType:FileAck)�� Ŭ���̾�Ʈ���� ���� */ 
 uint8_t msgType = FileAck; 
 ssize_t numBytesSent = send (clnt_sock, &msgType, sizeof (msgType), 0); 
 if (numBytesSent == -1) 
 error_handling ("send() error"); 
 else if (numBytesSent != sizeof (msgType)) 
 error_handling ("sent unexpected number of bytes"); 
 } 
  
 void FileDownloadProcess (int clnt_sock) 
 { 
  
 /* ���� �̸��� ���� : �ʵ� ũ��� 256Bytes�� ���� */ 
 char fileName[FILENAMESIZE]; 
 ssize_t numBytesRcvd = recv (clnt_sock, fileName, FILENAMESIZE, MSG_WAITALL); 
 if (numBytesRcvd == -1) 
 error_handling ("recv() error"); 
 else if (numBytesRcvd == 0) 
 error_handling ("peer connection closed"); 
 else if (numBytesRcvd != FILENAMESIZE) 
 error_handling ("recv unexpected number of bytes"); 
  
 /* ����ũ�⸦ Ŭ�� ���� : �ʵ� ũ��� uint32_t�� ���� */ 
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
  
 /* ���ϳ����� Ŭ�� ���� : �ʵ� ũ��� �� fileSize ������ */ 
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
 
 /* �������ۿϷ� */ 
 
 /* Ŭ���̾�Ʈ�κ����� ACK�޽��� ���� �� ȭ�鿡 �������� ��� */
 
 uint8_t msgType; 
 numBytesRcvd = recv (clnt_sock, &msgType, sizeof (msgType), MSG_WAITALL); 
 if (numBytesRcvd == -1) 
  error_handling ("recv() error");
  else if (numBytesRcvd == 0)
  error_handling ("peer connection closed");
  else if (numBytesRcvd != sizeof (msgType))
  error_handling ("recv unexpected number of bytes");
   
  if (msgType == FileAck)
  printf ("%s ���� ���� ����!!(%u Bytes)\n", fileName, fileSize);
  else
  printf ("%s ���� ���� ����!!\n", fileName);
  }