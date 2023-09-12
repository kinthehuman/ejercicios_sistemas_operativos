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
	fprintf(stderr, "%s\n", "usage: sourcefiles path [path...]");
	exit(EXIT_FAILURE);
}

int
WRONG_ARGS(int argc)
{
	if (argc == 1) {
		return 1;
	} else {
		return 0;
	}
}

struct data {

	long c_files;
	long h_files;
	long bytes;

};

int
ITERATE_DIR(DIR * dir, char *dirpath, struct data *files);

int
CHECK_FILE(char *path, struct stat *filestat, struct data *files)
{

	char *last_point;

	if ((filestat->st_mode & S_IFMT) == S_IFDIR) {

		DIR *open_dir;

		char *dir = path;

		int close_dir;

		if ((open_dir = opendir(dir)) == NULL) {
			perror(dir);
			return 1;
		} else {

			int i = ITERATE_DIR(open_dir, dir, files);

			if ((close_dir = closedir(open_dir)) < 0) {
				perror(dir);
				return 1;
			}
			return i;

		}
	} else {
		last_point = strrchr(path, '.');

		if (last_point != NULL && *(last_point + sizeof(char)) == 'c'
		    && *(last_point + 2) == '\0') {
			files->bytes = files->bytes + filestat->st_size;
			files->c_files++;
		}
		if (last_point != NULL && *(last_point + 1) == 'h'
		    && *(last_point + 2) == '\0') {
			files->bytes = files->bytes + filestat->st_size;
			files->h_files++;
		}

	}

	return 0;

}

int
ITERATE_DIR(DIR * dir, char *dirpath, struct data *files)
{

	struct dirent *entry = malloc(sizeof(struct dirent));
	struct stat *filestat = malloc(sizeof(struct stat));
	char *path = malloc(1000);
	int error = 0;

	while ((entry = readdir(dir))) {
		if (strcmp(".", entry->d_name) != 0
		    && strcmp("..", entry->d_name) != 0) {

			path = strcpy(path, dirpath);
			path = strcat(path, "/");
			path = strcat(path, entry->d_name);

			stat(path, filestat);

			//printf("checking %s\n", path);
			int e = CHECK_FILE(path, filestat, files);

			if (e == 1) {
				error = 1;
			}
		}
	}

	free(entry);
	free(filestat);
	free(path);

	return error;
}

int
main(int argc, char *argv[])
{
	if (WRONG_ARGS(argc)) {
		USAGE();
	}

	int error = 0;

	for (int i = 1; i < argc; i++) {

		struct data *files = malloc(sizeof(struct data));

		files->c_files = 0;
		files->h_files = 0;
		files->bytes = 0;

		DIR *open_dir;

		char *dir = argv[i];

		int close_dir;

		if ((open_dir = opendir(dir)) == NULL) {
			perror(dir);
			error = 1;
		} else {

			int e = ITERATE_DIR(open_dir, dir, files);

			if (e == 1) {
				error = 1;
			}
			if ((close_dir = closedir(open_dir)) < 0) {
				perror(dir);
				error = 1;
			}
		}

		printf("%s	%ld	%ld	%ld\n", argv[i], files->c_files,
		       files->h_files, files->bytes);

		free(files);
	}

	if (error == 1) {
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);

}
