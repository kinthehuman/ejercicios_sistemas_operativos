#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void
USAGE()
{
	fprintf(stderr, "%s\n",
		"usage: ch varname varcontent [varname varcontent] ...");
	exit(EXIT_FAILURE);
}

int
WRONG_ARGC(int argumentos)
{
	if (argumentos == 1 || argumentos % 2 == 0) {
		return 1;
	} else {
		return 0;
	}
}

int
COUNT_ERRORS(int size, char *pairs[])
{

	int err_count = 0;

	for (int i = 1; i < size; i = i + 2) {

		if (getenv(pairs[i]) == NULL
		    || strcmp(getenv(pairs[i]), pairs[i + 1]) != 0) {
			err_count++;
		}
	}
	return err_count;
}

void
DETECT_WRONG_PAIRS(int size, char *pairs[], int errors[])
{

	int j = 0;

	for (int i = 1; i < size; i = i + 2) {

		if (getenv(pairs[i]) == NULL
		    || strcmp(getenv(pairs[i]), pairs[i + 1]) != 0) {
			errors[j] = i;
			j++;
		}
	}
}

void
PRINT_ERRORS(char *pairs[], int errors[], int err_size)
{
	int index;

	fprintf(stderr, "%s", "error: ");
	for (int i = 0; i < err_size; i++) {
		index = errors[i];
		fprintf(stderr, "%s", pairs[index]);
		fprintf(stderr, "%s", " != ");
		fprintf(stderr, "%s", pairs[index + 1]);
		if (i < (err_size - 1)) {
			fprintf(stderr, "%s", ", ");
		} else {
			fprintf(stderr, "\n");
		}
	}
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{

	if (WRONG_ARGC(argc)) {
		USAGE();
	}

	int err_count = COUNT_ERRORS(argc, argv);

	malloc(err_count * sizeof(int));

	int errors_index[err_count];

	DETECT_WRONG_PAIRS(argc, argv, errors_index);

	if (err_count > 0) {
		PRINT_ERRORS(argv, errors_index, err_count);
	}

	//printf("%s\n", "check");

	exit(EXIT_SUCCESS);
}
