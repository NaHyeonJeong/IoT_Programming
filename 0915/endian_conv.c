#include<stdio.h> 
#include<stdlib.h> 
#include<string.h> 
#include<unistd.h> 
#include<arpa/inet.h> 
#include<sys/types.h> 
#include<sys/socket.h>


int main(int argc,char **argv)
{
    short host_port_order=0x1234;
    short net_port_order;

    long host_add_order=0x12345678;
	long net_add_order;

	net_port_order=htons(host_port_order);
	net_add_order=htonl(host_add_order);

	printf("Host ordered port : %x\n", host_port_order);
	printf("Network orderd port : %x\n\n", net_port_order);

	printf("Host ordered Address : %x\n",(unsigned int)host_add_order);
	printf("Network ordered Address : %x \n\n",(unsigned int)net_add_order);
	
	if(host_port_order == net_port_order)
	    printf("My System is Big-Endian Policy\n");
	else
	    printf("My System is Little-Endian Policy\n");

	return 0;
}

