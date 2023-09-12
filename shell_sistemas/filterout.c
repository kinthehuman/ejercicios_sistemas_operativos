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
	fprintf(stderr, "%s\n", "usage: filterout regexp command [args...]");
	exit(EXIT_FAILURE);
}

void
ARGS_CHECK(int argc, char *argv[])
{

	int max_args = 1024;
	int max_size = 1024;

	if (argc < 2) {
		USAGE();
	}
	if (argc > max_args) {
		fprintf(stderr, "usage: max arguments exceeded (%d)\n",
			max_args);
		exit(EXIT_FAILURE);
	}
	for (int i = 1; i < argc; i++) {
		if (strlen(argv[i]) > max_size) {
			fprintf(stderr, "usage: argument size exceeded (%d)\n",
				max_size);
			exit(EXIT_FAILURE);
		}
	}
}

void
FORK_AND_PIPE(int argc, char *argv[]){
	
	int fd[2];
	char **args = malloc((argc - 1) * sizeof(char *));
	char* path[1024] = "/bin";
	if(pipe(fd) != 0){
		err(EXIT_FAILURE, "pipe failed");
	}
	
	int pid = fork();

	switch (pid) {

	case -1:
		err(EXIT_FAILURE, "fork failed");
	case 0:
		
		close(fd[0]);
		if (dup2(fd[1], fileno(stdout)) == -1) {
			perror("cannot redirect stdout"); return 255;
		}
		
		for(int i = 2; i < argc){
			args[i-2] = argv[i];
		}
		
		args[argc-1] = NULL;
		
		strcat(path, args[0]);
		
		if(execv(path, args) != 0){
			path = "/bin/usr";
			strcat(path, args[0]);
			execv(path, args);
		};
		
		err(EXIT_FAILURE, "errno: %d", errno);

	default:
		close(fd[1]);
		if (dup2(fd[0], fileno(stdin)) == -1) {
			perror("cannot redirect stdin"); return 255;
		}
		wait(&status);
		if (WEXITSTATUS(status) != EXIT_SUCCESS) {
			fprintf(stderr, "filterout: program failed %s\n", args[0]);
			exit(EXIT_FAILURE);
		}
	}

}
