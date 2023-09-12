#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct ENV_VAR ENV_VAR;
struct ENV_VAR {
	char *name;
	char *value;
};

void
USAGE()
{
	fprintf(stderr, "%s\n", "usage: defenv [VAR=VAL...] path [args...]");
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

int
FIND_ENV(int argc, char *argv[])
{

	int N_enviroment = 0;

	for (int i = 1; i < argc; i++) {

		if (strchr(argv[i], '=') != NULL) {
			N_enviroment = i;
		}
	}

	return N_enviroment;
}

void
FORK_AND_WAIT_ENV(int argc, char *argv[], int N_enviroment)
{

	int N_path = N_enviroment + 1;

	if (N_path >= argc) {
		USAGE();
	}

	int N_args = argc - N_path;

	ENV_VAR *vars = malloc(N_enviroment * sizeof(ENV_VAR));

	if (vars == NULL && N_enviroment != 0) {
		fprintf(stderr, "%s\n", "defenv: malloc error");
		exit(EXIT_FAILURE);
	}

	char **args = malloc((N_args + 2) * sizeof(char *));

	if (args == NULL) {
		fprintf(stderr, "%s\n", "defenv: malloc error");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < N_enviroment; i++) {
		vars[i].name = strtok(argv[i + 1], "=");
		vars[i].value = strtok(NULL, "\0");
	}

	args[0] = argv[N_path];
	
	for (int i = 1; i < N_args; i++) {
		args[i] = argv[N_path + i];
		
	}

	args[N_args] = NULL;
	int status;
	int env;

	int pid = fork();

	switch (pid) {

	case -1:
		err(EXIT_FAILURE, "fork failed");
	case 0:
		
		for (int i = 0; i < N_enviroment; i++) {
			
			env = setenv(vars[i].name, vars[i].value, 0);
			if (env != 0) {
				err(EXIT_FAILURE, "setenv %s, errno: %d",
				    vars[i].name, errno);
			}

		}
		execv(args[0], args);
		err(EXIT_FAILURE, "errno: %d", errno);

	default:
		wait(&status);
		if (WEXITSTATUS(status) != EXIT_SUCCESS) {
			fprintf(stderr, "defenv: program failed %s\n", args[0]);
			exit(EXIT_FAILURE);
		}
	}
	free(vars);
	free(args);
}

int
main(int argc, char *argv[])
{
	ARGS_CHECK(argc, argv);

	int N_enviroment = FIND_ENV(argc, argv);

	FORK_AND_WAIT_ENV(argc, argv, N_enviroment);

	exit(EXIT_SUCCESS);
}
