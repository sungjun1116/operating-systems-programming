#include <stdio.h>
#include <stdlib.h>    //for system(exit() etc...)
#include <unistd.h>    //for execve(), fork() 
#include <string.h>  
#include <sys/wait.h>  //for wait()
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>   //for boolean type
#include <time.h>      //for time function

char* tokens[256];

//명령어 토큰으로 나누기
int tokenize(char* buf, int maxTokens)
{
	int token_count = 0;
	tokens[token_count] = strtok(buf," 	\n");
	
	while(tokens[token_count] != NULL && token_count<maxTokens)
	{
		tokens[++token_count] = strtok(NULL," 	\n");
	}
	return token_count;
}

//Chage Directory
void changeDir()
{
	if(!tokens[1])
		chdir(getenv("HOME"));
	else if(tokens[1])
		chdir(tokens[1]);
	else
	{
		printf("USAGE : cd directory_name \n");
	}
}

bool run(char* line)
{
	int token_count;
	int status;
	char* cut_path;       //PATH 경로를 parsing 하고 그 결과를 받는 포인터
    	char copy_path[500];  //PATH 경로를 받아오는 문자형 배열
    	char new_path[500];   //PATH 경로를 수정하여 저장할 문자형 배열

	pid_t pid;  //ID값을 저장할 변수
	token_count = tokenize(line,sizeof(tokens)/sizeof(char*));
	
	if(token_count=0)
		return true;
	
	if(strcmp(tokens[0],"quit")==0)
	{
		printf("------------!!quit!!------------\n");
		return false;  
	}
	
	if(strcmp(tokens[0],"cd")==0)
	{
		changeDir();
		return true;
	}
	
	//PATH를 parsing
	strcpy(copy_path, getenv("PATH"));
	cut_path = strtok(copy_path, ":");  //:으로 parsing
	strcpy(new_path, cut_path);
	strcat(new_path, "/");
	strcat(new_path, tokens[0]);  //parsing한 PATH뒤에 명령어를 붙임
	strcat(new_path, "");
	
	// fork()로 프로세스 복제
	pid = fork();
	if(pid == -1)                     
	{
		perror("fork failed\n");  
		exit(1);
	}
	//child process 코드
	else if(pid == 0)                 
	{
		while(execve(new_path,tokens,NULL)== -1){
			if(!(cut_path = strtok(NULL, ":")))  //맞는 PATH가 모두 검색해도 없을 경우 탈출
				break;
			// 다시 명령어를 입력받기 위한 작업
			strcpy(new_path, cut_path);
			strcat(new_path, "/");
			strcat(new_path, tokens[0]);  
			strcat(new_path, "");
		}
        perror("execute error\n");  //오류 메세지 출력
		exit(1); // child process를 종료
	}
	//parent process 코드
	else                              
	{
		pid_t waitPid;
		waitPid = wait(&status);  //child process가 작업종료 될떄까지 기다린 후 동작
	}
	return true;
}

int main()
{
	char line[1024];
	
	printf("종료를 원하시면 quit을 입력하세요.\n");
	while(1)
	{
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		printf("%s@",getenv("USER"));  //USER prompt using getenv()
		printf("%s",getenv("PWD"));    //PWD prompt using getenv() 
		printf(" <%d-%d-%d %d:%d:%d>~$ ",tm.tm_year+1900, tm.tm_mon+1,  //TIME prompt
			   tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
		fgets(line,sizeof(line),stdin);
		if(run(line) == false)
			break;
	}
	return 0;
}
