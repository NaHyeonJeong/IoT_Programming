#include<stdio.h>
#include<arpa/inet.h>

int main(void)
{
	struct sockaddr_in addr1,addr2;
	char *str;
		
	addr1.sin_addr.s_addr=htonl(0x3d0aa8c0); // ip
	addr2.sin_addr.s_addr=htonl(0x10aa8c0); // router ip

	str=inet_ntoa(addr1.sin_addr);
	printf("Dotted-Decimal notation : %s\n",str);

	inet_ntoa(addr2.sin_addr);
	printf("Dotted-Decimal notation : %s \n\n",str);
	return 0;
}
