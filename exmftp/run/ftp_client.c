
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

 /* ��� �Ķ������ ����(argc)�� 5������ �˻� */
  if (argc != 3)
  {
  printf ("Usage : %s <Server Address> <Server Port>\n", argv[0]);
  exit (1);
  }
  
  char *servIP = argv[1];
  char operand[50];
  in_port_t servPort = atoi(argv[2]);

 /* socket() : ���ϻ��� */
 int sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
 if (sock == -1)
 error_handling ("socket() error");
 
 /* connect() : ������ ���� */
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
 printf("-       �� ��       -\n");
 printf("-    1. upload      -\n");
 printf("-    2. download    -\n");
 printf("-    3. exit        -\n");
 printf("---------------------\n");

 printf("����(����) : ");
 scanf("%d",&op);

 printf("----------------------\n\n");
 
 /* op�� ���� �б��Ͽ� ó�� */  
 if (op == 1)
 { 
 printf("�����̸�(Ȯ��������) : "); 
 scanf("%s",operand);
 FileUploadProcess (sock, operand); 
 }
 else if (op == 2)
 {
 printf("�����̸�(Ȯ��������) : ");
 scanf("%s",operand);
 HandleFileDownload (sock, operand);
 }
 else if (op ==3)
  {
 
 /* msgType ���� : �ʵ� ũ��� 1Bytes(uint8_t)�� ���� */

  uint8_t msgType = ExitReq;
  ssize_t numBytesSent = send (sock, &msgType, sizeof (msgType), 0);
  if (numBytesSent == -1)
  error_handling ("send() error");
  else if (numBytesSent != sizeof (msgType))
  error_handling ("sent unexpected number of bytes");
  break;
  }

  else
  printf ("���� �޴���ȣ�Դϴ�. \n\n");

 } 

 /* close() : ���� ���� */

 close (sock); 
 return 0;
 }
 
 void FileUploadProcess (int sock, char *A_fileName)
 {

 /* msgType ���� : �ʵ� ũ��� 1Bytes(uint8_t)�� ���� */
 uint8_t msgType = FileUpReq;
 ssize_t numBytesSent = send (sock, &msgType, sizeof (msgType), 0);
 if (numBytesSent == -1)
 error_handling ("send() error");
 else if (numBytesSent != sizeof (msgType))
 error_handling ("sent unexpected number of bytes");
 
 /* �����̸��� ������ ���� : �ʵ� ũ�⸦ 256Bytes�� ���� */
 char fileName[FILENAMESIZE];
 memset (fileName, 0, FILENAMESIZE);
 strcpy (fileName, A_fileName);
 numBytesSent = send (sock, fileName, FILENAMESIZE, 0);
 if (numBytesSent == -1)
 error_handling ("send() error");
 else if (numBytesSent != FILENAMESIZE)
 error_handling ("sent unexpected number of bytes");
 
 /* ����ũ�⸦ ������ ���� : �ʵ� ũ��� uint32_t�� ���� */
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
 
 /* ���ϳ����� ������ ���� : �ʵ� ũ��� �� fileSize ������ */
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
 fclose (fp); /* �������ۿϷ� */
 
 /* �����κ����� ACK�޽��� ���� �� ȭ�鿡 �������� ��� */
 ssize_t numBytesRcvd = recv (sock, &msgType, sizeof (msgType), MSG_WAITALL);
 if (numBytesRcvd == -1)
 error_handling ("recv() error");
 else if (numBytesRcvd == 0)
 error_handling ("peer connection closed");
 else if (numBytesRcvd != sizeof (msgType))
 error_handling ("recv unexpected number of bytes");
 
 if (msgType == FileAck)
 printf ("%s ���ε� ����!!(%u Bytes)\n", fileName, fileSize);
 else
 printf ("%s ���� ���ε� ����!!\n", fileName);
 }
 
 void HandleFileDownload (int sock,char *A_fileName)
 {
 /* msgType ���� : �ʵ� ũ��� 1Bytes(uint8_t)�� ���� */
 uint8_t msgType = FileDownReq;
 ssize_t numBytesSent = send (sock, &msgType, sizeof (msgType), 0);
 if (numBytesSent == -1)
 error_handling ("send() error");
 else if (numBytesSent != sizeof (msgType))
 error_handling ("sent unexpected number of bytes");

 /* �����̸��� ������ ���� : �ʵ� ũ�⸦ 256Bytes�� ���� */
 char fileName[FILENAMESIZE];
 memset (fileName, 0, FILENAMESIZE);
 strcpy (fileName, A_fileName);
 numBytesSent = send (sock, fileName, FILENAMESIZE,0);
 if (numBytesSent == -1)
 error_handling ("send() error");
 else if (numBytesSent != FILENAMESIZE)
 error_handling ("sent unexpected number of bytes");
 
 /* ���� ũ�⸦ ���� : �ʵ� ũ��� uint32_t�� ���� */
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
 

 /* ���� ������ ���� : �ʵ� ũ��� �� uint32_t ���� */
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
 printf ("fileSize = %u\n�ٿ�ε� �Ϸ�^^\n", fileSize);
 fclose (fp); /* ���� ���� �Ϸ� */

 /* ���ϼ��� �����޽���(msgType:FileAck)�� �������� ���� */
 msgType = FileAck;
 numBytesSent = send (sock, &msgType, sizeof (msgType), 0);
 if (numBytesSent == -1)
 error_handling ("send() error");
 else if (numBytesSent != sizeof (msgType))
 error_handling ("sent unexpected number of bytes");
 }
