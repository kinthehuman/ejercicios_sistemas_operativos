#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct MACH MACH;
struct MACH {
	pid_t pid;
	char *name;
};

void
USAGE()
{
	fprintf(stderr, "%s\n", "usage: pinger N target [target...]");
	exit(EXIT_FAILURE);
}

void
ARGS_CHECK(int argc, char *argv[])
{

	int max_args = 20;
	int max_size = 50;

	if (argc < 3 || atoi(argv[1]) == 0) {
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

pid_t
FORK_AND_PING(char *n, char *target)
{

	int pid = fork();

	switch (pid) {

	case -1:
		err(EXIT_FAILURE, "fork failed");
	case 0:
		execl("/usr/bin/ping", "ping", "-c2", "-q", "-W", n, target,
		      NULL);
		err(EXIT_FAILURE, "errno: %d", errno);

	default:
		return pid;
	}
}

int
main(int argc, char *argv[])
{

	ARGS_CHECK(argc, argv);

	MACH *targets = malloc((argc - 2) * sizeof(MACH));
	int status;

	for (int i = 2; i < argc; i++) {
		targets[i - 2].pid = FORK_AND_PING(argv[1], argv[i]);
		targets[i - 2].name = argv[i];
	}

	for (int i = 0; i < (argc - 2); i++) {
		waitpid(targets[i].pid, &status, 0);
		if (WEXITSTATUS(status) != EXIT_SUCCESS) {
			fprintf(stderr, "\n\nnot found: %s\n", targets[i].name);
			free(targets);
			exit(EXIT_FAILURE);
		}
	}
	free(targets);

	exit(EXIT_SUCCESS);
}
