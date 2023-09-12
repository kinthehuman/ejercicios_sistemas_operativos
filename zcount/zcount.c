#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <dirent.h>

void
USAGE()
{
	fprintf(stderr, "%s\n", "usage: zcount dir");
	exit(EXIT_FAILURE);
}

int
WRONG_ARGS(int argc)
{
	if (argc == 2) {
		return 0;
	} else {
		return 1;
	}
}

int
OPEN_READ_FILE(char *file)
{

	int open_file = open(file, O_RDONLY);

	if (open_file < 0) {
		perror("open failure");
		exit(EXIT_FAILURE);
	}

	return open_file;
}

int
OPEN_WRITE_FILE(char *file)
{

	int open_file = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);

	if (open_file < 0) {
		perror(file);
		exit(EXIT_FAILURE);
	}

	return open_file;

}

long
COUNT_ZERO(int file)
{

	long zeros = 0;
	int bufsize = 2 ^ 1000;

	char *buffer = malloc(bufsize);
	int nr;

	while ((nr = read(file, buffer, bufsize)) != 0) {
		if (nr < 0) {
			err(EXIT_FAILURE, "read failure");
		} else {
			for (int i = 0; i < nr; i++) {
				if (buffer[i] == 0) {
					zeros++;
				}
			}
		}
	}

	free(buffer);
	return zeros;
}

void
ITERATE_DIR(DIR * dir, char *dirname)
{

	struct dirent *entry;
	struct stat filestat;
	long zeros;
	int read_file;
	char *path = malloc(100);

	path = strcpy(path, dirname);
	path = strcat(path, "/z.txt");
	FILE *write_file = fopen(path, "w");
	int close_file;

	while ((entry = readdir(dir))) {

		path = strcpy(path, dirname);
		path = strcat(path, "/");
		path = strcat(path, entry->d_name);

		stat(path, &filestat);

		if ((filestat.st_mode & S_IFMT) != S_IFDIR
		    && strcmp(entry->d_name, "z.txt") != 0) {

			read_file = OPEN_READ_FILE(path);

			zeros = COUNT_ZERO(read_file);

			//printf("%ld %s\n", zeros, entry->d_name);

			if ((close_file = close(read_file)) < 0) {
				perror(entry->d_name);
				exit(EXIT_FAILURE);
			}

			fprintf(write_file, "%ld %s\n", zeros, entry->d_name);
		}

	}

	free(path);

	if ((close_file = fclose(write_file)) < 0) {
		perror("z.txt");
		exit(EXIT_FAILURE);
	}
}

int
main(int argc, char *argv[])
{
	if (WRONG_ARGS(argc)) {
		USAGE();
	}

	DIR *open_dir;

	char *dir = argv[1];

	int close_dir;

	if ((open_dir = opendir(dir)) == NULL) {
		perror(dir);
		exit(EXIT_FAILURE);
	} else {

		ITERATE_DIR(open_dir, dir);

		if ((close_dir = closedir(open_dir)) < 0) {
			perror(dir);
			exit(EXIT_FAILURE);
		}
	}

	exit(EXIT_SUCCESS);

}
