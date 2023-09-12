#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


void
USAGE()
{
	fprintf(stderr, "%s\n",
		"usage: execargs secs command [command ...]");
	exit(EXIT_FAILURE);
}

int
WRONG_ARGS(int argcount, char *argumentos[])
{
	if (argcount <= 2 || !(atoi(argumentos[1]))) {
		return 1;
	} else {
		return 0;
	}
}

struct command{
	char *path;
	char *argv[];
};


void
FORK_AND_WAIT(unsigned int wait_time, char *argv)
{
	struct command instruction;
	
	malloc(strlen(argv));
	
	char dst[strlen(argv)];
	
	strcpy(dst, argv);
	
	char delim[] = " ";
	
	char *ptr = strtok(dst, delim);
	
	instruction.path = ptr;
	
	int i = 0;
	
	while (ptr != NULL){
		instruction.argv[i] = ptr;
		ptr = strtok(NULL, delim);
		i++;
	}
	
	int pid = fork();
	
	int status;
	
	switch(pid){
	
	case -1:
		err(EXIT_FAILURE, "fork failed");
	case 0:
		execv(instruction.path, instruction.argv);
		exit(EXIT_FAILURE);
		
	default:
		sleep(wait_time);
		wait(&status);
		if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS){
			err(EXIT_FAILURE, "Error: %s: ", argv);
		}
	}

}

int
main(int argc, char *argv[])
{
	if (WRONG_ARGS(argc, argv)) {
		USAGE();
	}
	
	for (int i = 2; i < argc; i++){
	
		FORK_AND_WAIT(atoi(argv[0]), argv[i]);
		
	}
	
	

	exit(EXIT_SUCCESS);
}
