#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>

void
USAGE()
{
	fprintf(stderr, "%s\n",
		"usage: copybytes source destination [max_bytes]");
	exit(EXIT_FAILURE);
}

int
WRONG_ARGS(int argc, char *argv[])
{
	if (argc == 4) {
		if (!(atoi(argv[3]))) {
			return 1;
		} else {
			return 0;
		}
	} else if (argc == 3) {
		return 0;
	} else {
		return 1;
	}
}

int
GET_MAXSIZE(int argc, char *argv[])
{
	int maxsize;

	if (argc == 4) {

		maxsize = atoi(argv[3]);

	} else {

		maxsize = -1;
	}

	return maxsize;

}

int
OPEN_READ_FILE(char *file)
{
	if (strcmp(file, "-") == 0) {
		return 0;
	} else {
		int open_file = open(file, O_RDONLY);

		if (open_file < 0) {
			perror(file);
			exit(EXIT_FAILURE);
		}
		return open_file;
	}
}

int
OPEN_WRITE_FILE(char *file)
{
	if (strcmp(file, "-") == 0) {
		return 1;
	} else {
		int open_file = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);

		if (open_file < 0) {
			perror(file);
			exit(EXIT_FAILURE);
		}
		return open_file;
	}
}

void
READ_AND_WRITE(int read_file, int write_file, int bufsize, int maxbytes)
{

	char *buffer = malloc(bufsize);
	int nr;
	int bytes = 0;
	int max_reached = 0;

	while ((nr = read(read_file, buffer, bufsize)) != 0 && !max_reached) {
		if (nr < 0) {
			err(EXIT_FAILURE, "read failure");
		}
		if (maxbytes > 0){
			bytes = bytes + nr;
			if (bytes >= maxbytes) {
				nr = nr - (bytes - maxbytes);
				max_reached = 1;
			}
		}
		if (write(write_file, buffer, nr) != nr) {
			err(EXIT_FAILURE, "write failure");
		}
	}

	free(buffer);

}

int
main(int argc, char *argv[])
{
	if (WRONG_ARGS(argc, argv)) {
		USAGE();
	}

	int bufsize = 32;

	int read_file = OPEN_READ_FILE(argv[1]);
	int write_file = OPEN_WRITE_FILE(argv[2]);
	int maxsize = GET_MAXSIZE(argc, argv);

	READ_AND_WRITE(read_file, write_file, bufsize, maxsize);

	if (read_file != 0) {
		read_file = close(read_file);
	}

	if (write_file != 1) {
		write_file = close(write_file);
	}

	exit(EXIT_SUCCESS);

}
