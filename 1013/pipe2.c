#include <stdio.h>
#include <unistd.h>
#define BUF_SIZE 30

int main(int argc, char *argv[])
{
	int fds[2];
	char str1[]="Who are you?";
	char str2[]="Thank you for your message";
	char buf[BUF_SIZE];
	pid_t pid;
	
	pipe(fds);
	pid=fork();
	// 양방향 통신을 하는 경우 데이터 읽고 쓰는 타이밍이 매우 중요해짐
	// sleep()이 없으면 문제 발생...
	if(pid==0) { // 자식 프로세스
		write(fds[1], str1, sizeof(str1));
		//sleep(2); // 부모가 읽을 수 있도록 기다림
		read(fds[0], buf, BUF_SIZE);
		printf("Child proc output : %s\n", buf);
	}
	else { // 부모 프로세스
		read(fds[0], buf, BUF_SIZE);
		printf("Parent proc output : %s\n", buf);
		write(fds[1], str2, sizeof(str2));
        //sleep(3); // 자식이 읽을 수 있도록 기다림
	}
	return 0;
}
