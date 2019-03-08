#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
int main (int argc, char* argv[])
{
	if (argc !=5){
		printf("par expects 4 arguments: par cmd1 arg1 cmd2 arg2\n");
		return 0;
	}
	pid_t child1 = fork();
	pid_t child2 = fork();
	if (child1 && child2){
		int status1,status2;
		int count = 0;
		waitpid(child1,&status1,0);
		waitpid(child2,&status2,0);
		printf("status:%d %d\n",status1,status2);
		if (WEXITSTATUS(status2) == 123) printf("Program %d, %s, could not be run\n",1,argv[1]);
		if (WEXITSTATUS(status1) == 123) printf("Program %d, %s, could not be run\n",2,argv[3]);
		if (status1 == 0) count++;
		if (status2 == 0) count++;
		printf("%d program(s) exited normally\n",count);
	}else if (child1){
			execlp(argv[1],argv[1],argv[2],(char*)NULL);
			//execlp("ls","ls",(char*)NULL);
			exit(123);
	}else if (child2){
			execlp(argv[3],argv[3],argv[4],(char*)NULL);
			exit(123);	
	}
	return 0;
}