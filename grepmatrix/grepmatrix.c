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
		"usage: grepmatrix word [words ...] -f file [file ...]");
	exit(EXIT_FAILURE);
}

int
MODIFIER_INDEX(int argcount, char *argumentos[])
{
	for (int i = 1; i < argcount; i++) {
		if (strcmp(argumentos[i], "-f") == 0) {
			return i;
		}
	}
	return 0;
}

int
WRONG_ARGS(int argcount, char *argumentos[])
{
	int modifier = 0;

	for (int i = 1; i < argcount; i++) {
		if (strcmp(argumentos[i], "-f") == 0) {
			if (i == 1 || i == argcount - 1 || modifier != 0) {
				return 1;
			} else {
				modifier++;
			}
		}
	}
	return !modifier;
}

int
SEARCH(char *word, char *file)
{

	int pid = fork();

	switch (pid) {

	case -1:
		err(EXIT_FAILURE, "fork failed");
	case 0:
		execl("/usr/bin/grep", "fgrep", "-q", "-s", word, file, NULL);
		exit(EXIT_FAILURE);

	default:
		return pid;
	}
}

typedef struct {

	char *word;
	char *file;
	int pid;
	int status;

} SEARCHER;

void
INIT_SEARCHERS(int argc, char *argv[], SEARCHER searchers[], int modifier)
{

	int k = 0;

	for (int i = 1; i < modifier; i++) {
		for (int j = modifier + 1; j < argc; j++) {
			searchers[k].word = (char *)malloc(sizeof(char *));
			searchers[k].word = argv[i];
			searchers[k].file = (char *)malloc(sizeof(char *));
			searchers[k].file = argv[j];
			searchers[k].pid =
			    SEARCH(searchers[k].word, searchers[k].file);
			k++;
		}
	}

}

void
WAIT_FOR_SEARCHERS(SEARCHER searchers[], int searcher_count)
{

	int status;
	int pid;

	while ((pid = wait(&status)) != -1) {
		for (int i = 0; i < searcher_count; i++) {
			if (WIFEXITED(status) && pid == searchers[i].pid) {
				searchers[i].status = WEXITSTATUS(status);
			}
		}
	}
}

void
PRINT_ERRORS(SEARCHER searchers[], int searcher_count)
{

	for (int i = 0; i < searcher_count; i++) {
		if (searchers[i].status == 2) {
			fprintf(stderr,
				"error processing file %s\n",
				searchers[i].file);
			exit(EXIT_FAILURE);
		}
	}
}

void
PRINT_MATRIX(int argc, char *argv[], SEARCHER searchers[], int modifier,
	     int files)
{

	for (int j = modifier; j < argc; j++) {
		for (int i = 1; i < modifier; i++) {
			if (j == modifier) {
				printf("%c%s%c%c", '"', argv[i], '"', 9);
			} else {
				int index =
				    (j - modifier - 1) + (i - 1) * files;
				if (searchers[index].status == 0) {
					printf("%c%c", 'x', 9);
				}
				if (searchers[index].status == 1) {
					printf("%c%c", 'o', 9);
				}
			}
		}
		if (j != modifier) {
			printf("%s", argv[j]);
		}
		printf("\n");
	}

}

int
main(int argc, char *argv[])
{
	if (WRONG_ARGS(argc, argv)) {
		USAGE();
	}

	int modifier = MODIFIER_INDEX(argc, argv);
	int words = modifier - 1;
	int files = argc - modifier - 1;
	int searcher_count = words * files;
	SEARCHER *searchers = malloc(searcher_count * sizeof *searchers);

	INIT_SEARCHERS(argc, argv, searchers, modifier);

	WAIT_FOR_SEARCHERS(searchers, searcher_count);

	PRINT_ERRORS(searchers, searcher_count);

	PRINT_MATRIX(argc, argv, searchers, modifier, files);

	exit(EXIT_SUCCESS);
}
