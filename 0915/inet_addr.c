#include<stdio.h>
#include<arpa/inet.h>

int main(int argc, char **argv)
{
	char addr1[30]="";
	char addr2[30]="";
	unsigned long conv_addr;
	
	printf("Your PC IP > ");
	scanf("%s", addr1);
	printf("Your Internet Router IP > ");
    scanf("%s", addr2);
    
	conv_addr=inet_addr(addr1);
	if(conv_addr==INADDR_NONE)
        printf("Error Occur : %ld \n",conv_addr);
	else
        printf("Unsigned long addr(network ordered):%lx\n",conv_addr);
    
	conv_addr=inet_addr(addr2);
	if(conv_addr==INADDR_NONE)
		printf("Error Occured %ld \n\n",conv_addr);
	else
		printf("unsigned long addr(nerwork ordered):%lx\n\n",conv_addr);
	return 0;
}			

