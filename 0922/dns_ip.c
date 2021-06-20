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

void main(int argc,char *argv[])
{
	struct hostent *myhost;
	struct in_addr myinaddr; //IP 주소를 저장할 구조체 
	int i;
	
	if(argc<2)
	{
	printf("Usage: %s host_name(domain name)\n",argv[0]);
	exit(1);
    }
	// hostent 구하기
	myhost=gethostbyname(argv[1]);
	if(myhost==0)
    {
        printf("erro occurs .. at 'gethostbyname'.\n\n\n");
        exit(1);
    }

    // 호스트 이름 출력
    printf("official host name : \t\t %s\n",myhost->h_name);
	i=0;
	//호스트 별명 출력
	while(myhost->h_aliases[i]!=NULL)
	{
		printf("aliases name : \t\t%s\n",myhost->h_aliases[i]);
		i++;
	}
	
    //호스트 주소체계 출력
	printf("host address type : \t\t%d\n",myhost->h_addrtype);
	//호스트 주소 길이 출력
	printf("length of host address : \t%d\n",myhost->h_length);
	//호스트 주소를 dotted decimal 형태로 출력 
	i=0;
	while(myhost->h_addr_list[i]!=NULL)
	{
		myinaddr.s_addr=*((u_long *)(myhost->h_addr_list[i]));
		printf("address for host:\t\t%s\n",inet_ntoa(myinaddr));
		i++;
	}
}		

