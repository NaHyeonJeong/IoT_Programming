/**************************************************************************************************
작 성 자 : 컴퓨터공학부 201858042 나현정
작 성 일 : 2020.10.05
프로그램 : Small DNS
설    명 : 클라이언트로 부터 쿼리를 받아(무한루프) 캐시 배열에 쿼리에 해당되는 데이터가 있는지 
먼저 찾는다. 있으면 그 데이터를 클라이언트에 보내주고 없으면 데이터들이 저장되어 있는 트리에 가서 
찾는다. 트리에 있으면 해당 데이터를 클라이언트에 보내고 hit값을 캐시의 마지막에 들어있는 값과 
비교한다. 트리에 없으면 외부에서 데이터를 가져와서 클라이언트에 보내고 트리에 insert한다.
**************************************************************************************************/
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
#include"rbt.h"
#define MAX_NODE		20000	// 처리가능 dns 수
#define MAX_CACHE		10		// 캐시 수
#define MAX_DATA_LEN	150

typedef struct cache_{
    char key[MAX_DATA_LEN]; // domain or ip
	char data[MAX_DATA_LEN]; // ip or domain
	int hitCnt; // hit count
} Cache;

void error_handling(char *message);
int my_compare(const void *a, const void *b);
int my_compare_num(const void *a, const void *b);
void inorderR(Node *node);
void inorder(RBT tree);
void insertion_sort(Cache list[], int n);
void updateCache(Cache cacheArr[], int sizeArr, char *key, char *value1, int hitCnt);

int main(int argc, char **argv) {
	int serv_sock;
    int clnt_sock;
    int str_len;
    char query[MAX_NODE][MAX_DATA_LEN]; // client query
    char data[MAX_NODE][MAX_DATA_LEN]; // query에 대응하는 data
   	struct sockaddr_in serv_addr;
   	struct sockaddr_in clnt_addr;
 	int clnt_addr_size;
 	int qnum = 0;
    
	unsigned int conv_addr;
    struct hostent *myhost;
	struct hostent *myhost2;
   	struct sockaddr_in addr;
   	struct in_addr myinaddr;
   	int i, j; 
   	int hitCnt = 0; // 0
    RBT rbt;
    Cache cache [MAX_CACHE]; // Cache array
    int is_found = 0; // false
    Node *node;
	
	myhost = gethostbyname(""); // 이거 안하면 오류남.. 분석필요
    
    if(argc!=2) {
		printf("Usage: %s <port> \n",argv[0]);
		exit(1);
	}
	
	// 소켓
	serv_sock=socket(PF_INET,SOCK_STREAM,0); 
	if(serv_sock==-1) 
		error_handling("socket() error");
	
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
    
	if(bind(serv_sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr))==-1) 
		error_handling("bind() error");
	if(listen(serv_sock,5)==-1) 
		error_handling("listen() error");
	
    for(j = 0; j < MAX_CACHE; j++) { // cache 배열 초기화
        strcpy(cache[j].key, "");
        strcpy(cache[j].data, "");
        cache[j].hitCnt = j*-1;
    }
	rbt_init(&rbt, my_compare); // data 트리 초기화
    
	// accept와 close를 반복하면 클라이언트의 쿼리를 계속해서 받을 수 있음
	while(1) {
	    printf("Listening =============================================\n");
	    clnt_addr_size = sizeof(clnt_addr);
	    clnt_sock = accept(serv_sock,(struct sockaddr*)& clnt_addr,&clnt_addr_size); // 여기에서 리스닝 대기
	    
        // 수신된거 처리
	    if(clnt_sock == -1) {
			error_handling("accept() error");
			break;
		}
		
		// 반복해서 클라이언트와 통신하기 위해서 만든 while문
		while(str_len = read(clnt_sock, query[qnum], sizeof(query[qnum])-1)) {
			if(str_len == -1) 
				error_handling("read() error!");
			query[qnum][str_len] = 0;
			printf("Query from client > %s \n", query[qnum]); // 두 번씩 출력됨... 왜지...?
				
			conv_addr = inet_addr(query[qnum]);
			
			// search query from cache array
			printf("==== Start searching domain from cache array\n");

			for(j = 0; j < MAX_CACHE; j++) {
				/////////////////////////////////////////
				// cache 에서 query를 검색
				/////////////////////////////////////////
				if(strcmp((char *)cache[j].key, query[qnum]) == 0) {
					printf("Found in cache =================== %s\n", (char *)cache[j].key);
					// 클라이언트에 write
					write(clnt_sock, (char *)cache[j].data, strlen(cache[j].data));
					
					cache[j].hitCnt++; // hit count + 1
					insertion_sort(cache, MAX_CACHE);
					
					is_found = 1;
					break;
				}

			} // end of for j
			printf("==== End searching domain from cache array\n");

			if(!is_found) {
				/////////////////////////////////////////
				// cache array에서 query를 못찾음
				// 트리에서 찾아보기 
				/////////////////////////////////////////
				printf("==== %s not found in cache!!\n",(char *)query[qnum]);
				node = search(rbt, query[qnum]);
				
				if(node == NULL) { // 트리에서 못 찾음
					printf("==== %s not found in data tree!!\n",(char *)query[qnum]);
					// go to remote server ... get data
					if(conv_addr == INADDR_NONE) { // client query = host name(domain)
						printf("[[[ Using gethostbyname ]]]\n");
						myhost = gethostbyname(query[qnum]);
					}
					else{ // client query = ip
						printf("[[[ Using gethostbyaddr ]]]\n");
						memset(&addr, 0, sizeof(addr));
						addr.sin_addr.s_addr = conv_addr;
						myhost = gethostbyaddr((char*)&addr.sin_addr, 4, AF_INET);
					}
					
					if(myhost == 0) { // 외부에서 못 받아옴 (없는 host name or ip)
						printf("error occurs .. at 'gethost' -- Not found.\n");
						char *notFoundStr;
						notFoundStr = "Not found!!!";
						write(clnt_sock, notFoundStr, strlen(notFoundStr));
					}
					else{ // 외부에서 잘 받아옴
						if(conv_addr == INADDR_NONE) { // query가 host name(domain)인 경우
							i=0;
							while(myhost->h_addr_list[i]!=NULL) {
								// 외부에서 받아온 데이터를 트리에 넣고 클라이언트에 전송
								// 복수 ip 나와도 1개만 처리하고 break
								myinaddr.s_addr=*((u_long *)(myhost->h_addr_list[i]));
								strcpy(data[qnum], inet_ntoa(myinaddr));
								insert(&rbt, query[qnum], data[qnum], hitCnt = 1);
								printf("==== Add node to data tree |%s|%s\n",(char *)query[qnum], inet_ntoa(myinaddr));
								write(clnt_sock, inet_ntoa(myinaddr), strlen(inet_ntoa(myinaddr)));
								
								i++;
								break;
							}
						}
						else{  // query가 host ip인 경우
							i=0;
							while(myhost->h_addr_list[i]!=NULL) {
								strcpy(data[qnum], myhost->h_name);
								insert(&rbt, query[qnum], data[qnum], hitCnt = 1);
								printf("==== Add node to data tree |%s|%s\n",(char *)query[qnum], myhost->h_name);
								write(clnt_sock, myhost->h_name, strlen(myhost->h_name));
								
								i++;
								break;
							}
						}
					}
				}
				else { // 트리에서 찾았음
					printf("==== Found in data tree ==== %s|%s|%s|%d\n", (char *)query[qnum], (char *)node->key, (char *)node->value1, (int)node->value2);
					// 클라이언트에 찾은 ip write
					write(clnt_sock, (char *)node->value1, strlen((char *)node->value1));
					node->value2++;
					// 캐시 배열에 data tree에서 찾은 domain 값에 해당되는 hit값을 캐시 배열의 마지막 인덱스의 hit값과 비교해서
					// 넣을 수 있는지 확인, 필요하면 캐시 배열의 값을 교환
					updateCache(cache, MAX_CACHE, (char *)node->key, (char *)node->value1, (int)node->value2);
				}
			}

		}
        //str_len = read(clnt_sock, query[qnum], sizeof(query[qnum])-1); // serv_sock
        
	    qnum++;
		is_found = 0;
       	close(clnt_sock);
	}
	
   	printf("main 종료 ===================================\n");
    return 0;
}

void error_handling(char *message) { 
	fputs(message, stderr); 
	fputc('\n', stderr);
	exit(1);
}
int my_compare(const void *a, const void *b) {
	return strcmp((char *)a, (char *)b);
}
// 중위순회
void inorderR(Node *node) { // 실질적인 중위 순회 기능을 하는 함수
	if (node == NULL) return;
	inorderR(node->left);
    char str[200] = {0};
    strcat(str,node->key);
    strcat(str,"|");
    strcat(str,node->value1);
    puts(str);
	inorderR(node->right);
}
void inorder(RBT tree) { // 호출 할 때 사용할 함수
	inorderR(tree.root);
}
// insertion sort(삽입 정렬) : 내림차순
void insertion_sort(Cache list[], int n) {
    int i, j;
    Cache k; // 값 소실 방지용

    for (i = 1; i < n; i++) {
        k = list[i];
        j = i-1;
        while (j >= 0 && list[j].hitCnt < k.hitCnt) { // 내림차순
            list[j+1] = list[j];
            j--;
        }
        list[j+1] = k;
    }
    for(i = 0; i < n; i++)
        printf("i=%d | %s | %s | %d\n",i, (char *)list[i].key, (char *)list[i].data, list[i].hitCnt);
}
void updateCache(Cache cacheArr[], int sizeArr, char *key, char *value1, int hitCnt) {
    printf("===== Start updating last cache item\n");
    if(cacheArr[sizeArr - 1].hitCnt < hitCnt) {
		strcpy(cacheArr[sizeArr - 1].key, key);
        strcpy(cacheArr[sizeArr - 1].data, value1);
        cacheArr[sizeArr - 1].hitCnt = hitCnt;
    }
	insertion_sort(cacheArr, MAX_CACHE);
	printf("===== End updating last cache item\n");
}
