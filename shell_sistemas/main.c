#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <err.h>
#include <signal.h>
#include <poll.h>

#define max_inputsize 1024
#define max_comms 20
#define max_args 20

typedef struct ENV_VAR ENV_VAR;
struct ENV_VAR {
	char *name;
	char *value;
};

typedef struct parsed_input parsed_input;
struct parsed_input {
	char *infile;
	char *outfile;
	int bg;
	int comc;
	int error;
	char *commands[max_comms];

};

typedef struct command command;
struct command {
	char *com;
	char *infile;
	char *outfile;
	int fdin;
	int fdout;
	int bg;
	int error;
	int argc;
	int first;
	int last;
	int i;
	char *args[max_args + 1];
};

int
read_input(char *input)
{

	int done = 0;

	if (fgets(input, max_inputsize, stdin) == NULL) {
		done = 1;
	}

	return done;
}

void
remove_spaces(char *s)
{
	char *d = s;

	do {
		while (*d == ' ') {
			++d;
		}
	}
	while ((*s++ = *d++));
}

void
remove_tabs(char *s)
{
	char *d = s;

	do {
		while (*d == '\t') {
			++d;
		}
	}
	while ((*s++ = *d++));
}

parsed_input *
parse_input(char *input)
{

	size_t l = strlen(input);
	int bg = 0;
	int comc = 0;
	char *infile;
	char *outfile;
	char *commands[20];
	char *comstr;
	parsed_input *result = malloc(sizeof(parsed_input));

	result->error = 0;

	//DISCARDS SHORT READS

	if (l <= 1 || input[l - 1] != '\n') {
		result->error = 3;
		return result;
	}
	//BACKGROUND REQUEST DETECTION

	if (strrchr(input, '&') == &input[l - 2]) {
		bg = 1;
		input = strtok(input, "&");
	} else {
		input = strtok(input, "\n");
	}

	//AVOIDS PARSING COLLAPSE
	l = strlen(input);
	if (strrchr(input, '<') == &input[l - 1]
	    || strrchr(input, '<') == &input[0]) {
		result->error = 1;
		return result;
	}
	//INFILE AND OUTFILE DETECTION

	infile = strchr(input, '<');
	if (infile != NULL && strlen(infile) < 1) {
		result->error = 3;
		return result;
	}
	outfile = NULL;
	if (infile != NULL) {

		//printf("Infile detected\n");

		input = strtok(input, "<");
		infile = strtok(NULL, "<");
		outfile = strchr(infile, '>');

		if (outfile != NULL) {

			//printf("Outfile detected after infile\n");

			infile = strtok(infile, ">");
			outfile = strtok(NULL, ">");
		}
	}
	if (outfile == NULL) {
		outfile = strchr(input, '>');
		if (outfile != NULL) {

			//printf("Outfile detected\n");

			input = strtok(input, ">");
			outfile = strtok(NULL, "<");
		}
	}
	//END OF INFILE AND OUTFILE DETECTION

	//PARSING OF COMMANDS

	comstr = strtok(input, "|");

	while (comstr != NULL) {
		commands[comc] = comstr;
		comstr = strtok(NULL, "|");
		comc++;
		if (comc >= max_comms) {
			result->error = 2;
			return result;
		}
	}
	//RETURN VALS
	if (infile != NULL) {
		remove_spaces(infile);
		remove_tabs(infile);
	}

	if (outfile != NULL) {
		remove_spaces(outfile);
		remove_tabs(outfile);
	}

	if (bg == 1 && infile == NULL) {
		infile = "/dev/null";
	}
	result->infile = infile;
	result->outfile = outfile;
	result->bg = bg;
	result->comc = comc;

	for (int i = 0; i < comc; i++) {
		result->commands[i] = commands[i];
	}

	return result;

}

command *
generate_command(parsed_input *in, int index)
{

	command *result = malloc(sizeof(command));
	int argc = 0;

	result->fdin = fileno(stdin);
	result->fdout = fileno(stdout);
	result->error = 0;
	//SEPARATES COMMAND AND ARGS
	char *argstr = strtok(in->commands[index], " ");

	result->com = argstr;
	result->i = index;
	while (argstr != NULL) {

		remove_spaces(argstr);
		remove_tabs(argstr);

		if (argstr[0] == '$') {
			if (getenv(argstr + sizeof(char)) == NULL) {
				fprintf(stderr,
					"Error: variable %s does not exist\n",
					argstr + sizeof(char));
				result->error = 1;
			} else {
				argstr = getenv(argstr + sizeof(char));
			}
		}
		result->args[argc] = argstr;
		argstr = strtok(NULL, " ");
		argc++;
		if (argc >= max_args) {
			result->error = 2;
			return result;
		}
	}

	result->args[argc] = NULL;
	result->argc = argc;

	//ASSINGS INPUT AND OUTPUT

	//CASE FIRST

	if (index == 0) {
		result->first = 1;
		result->infile = in->infile;
	} else {
		result->first = 0;
		result->infile = NULL;	//IF NOT FIRST, STDIN WILL BE REDIRECTED TO PIPE
	}

	//CASE LAST

	if (index == (in->comc - 1)) {
		result->last = 1;
		result->outfile = in->outfile;
	} else {
		result->last = 0;
		result->outfile = NULL;	//IF NOT LAST, STDIN WILL BE REDIRECTED TO PIPE
	}

	result->bg = in->bg;

	return result;
}

void
SET_ENV(char *com)
{

	char *cpy = malloc(1024 * sizeof(char));

	cpy = strcpy(cpy, com);
	char *name = strtok(cpy, "=");
	char *value = strtok(NULL, "\0");
	int env = setenv(name, value, 0);

	free(cpy);
	if (env != 0) {
		perror("setenv failed");
		exit(EXIT_FAILURE);
	}
}

char *
LOOK_FOR_EXECUTABLE(char *c)
{
	//TRIES TO ACCESS THE FILE IN /bin, the working direcotry, or a directory found in PATH

	//CHECKS /bin
	char *fname = malloc(1024 * sizeof(char));

	fname = strcpy(fname, "/bin/");
	fname = strcat(fname, c);

	if (access(fname, X_OK) != 0) {
		//CHECKS cwd
		fname = getcwd(fname, 1024);
		fname = strcat(fname, "/");
		fname = strcat(fname, c);
	}
	//CHECKS $PATH
	if (access(fname, X_OK) != 0) {

		char *path = "PATH";
		char *path_dirs[150];

		path = getenv(path);

		char *dir = strtok(path, ":");
		int dirc = 0;

		while (dir != NULL) {
			path_dirs[dirc] = dir;
			dir = strtok(NULL, ":");
			dirc++;
		}

		int i = 0;

		while (access(fname, X_OK) != 0 && i < dirc) {
			fname = strcat(path_dirs[i], "/");
			fname = strcat(fname, c);
			i++;
		}
	}
	if (access(fname, X_OK) == 0) {
		return fname;
	} else {
		fprintf(stderr, "Error: could not find executable: %s\n", c);
		return NULL;
	}
}

int
DUP_INPUT_FD(command *c, int pipes[][2])
{
	int fd = fileno(stdin);

	if (c->first != 1) {
		fd = pipes[c->i - 1][0];
	} else if (c->infile != NULL) {
		fd = open(c->infile, O_RDONLY);

	} else {
		return fd;
	}

	if (dup2(fd, fileno(stdin)) == -1) {
		perror("cannot redirect stdin");
		exit(EXIT_FAILURE);
	} else {
		return fd;
	}
}

int
DUP_OUTPUT_FD(command *c, int pipes[][2])
{
	int fd = fileno(stdout);

	if (c->last != 1) {

		fd = pipes[c->i][1];
	} else if (c->outfile != NULL) {
		fd = open(c->outfile, O_WRONLY | O_CREAT | O_TRUNC);

	} else {
		return fd;
	}

	if (dup2(fd, fileno(stdout)) == -1) {
		perror("cannot redirect stdout");
		exit(EXIT_FAILURE);
	} else {
		return fd;
	}

}

pid_t
FORK_AND_RUN(command *c, int pipes[][2], int pipec)
{

	pid_t pid = fork();

	switch (pid) {

	case -1:
		perror("fork failed");
		return pid;
	case 0:

		//CLOSES ALL UNUSED PIPES
		for (int i = 0; i < pipec; i++) {
			if (i != c->i) {
				close(pipes[i][1]);
			}
			if (i != c->i - 1) {
				close(pipes[i][0]);
			}
		}
		//REDIRECTS STDIN AND STDOUT
		c->fdin = DUP_INPUT_FD(c, pipes);
		c->fdout = DUP_OUTPUT_FD(c, pipes);

		if (strchr(c->com, '=') == NULL && strcmp(c->com, "cd") != 0) {
			c->com = LOOK_FOR_EXECUTABLE(c->com);
			if (c->com == NULL) {
				exit(EXIT_FAILURE);
			}
			execv(c->com, c->args);
			err(EXIT_FAILURE, "errno: %d", errno);
		}

		exit(EXIT_SUCCESS);
	default:

		return pid;
	}
}

void
print_CWD()
{

	char cwd[1024];

	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		printf("%s$ ", cwd);
	} else {
		perror("getcwd() error");
	}

}

void
custom_stenv(command *c)
{

	if (c->argc > 1) {
		fprintf(stderr, "setenv error: too many arguments\n");
		exit(EXIT_FAILURE);
	}
	SET_ENV(c->com);

}

void
custom_CD(command *c)
{

	if (c->argc > 2) {
		fprintf(stderr, "cd error: too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (c->argc == 1) {
		c->args[1] = getenv("HOME");
	}

	if (chdir(c->args[1]) != 0) {
		perror("cd failed");
		exit(EXIT_FAILURE);
	}

}

int
execute_command(int bg, command *c, int pipec, int pipes[max_comms][2])
{
	//CHECKS IF COMMAND IS SETENV
	if (strchr(c->com, '=') != NULL) {
		custom_stenv(c);
	}
	//CHECKS IF COMMAND IS CD
	else if (strcmp(c->com, "cd") == 0) {
		custom_CD(c);
	}

	if (bg == 1) {
		pid_t pid = fork();

		switch (pid) {

		case -1:

			perror("fork failed");
			return pid;

		case 0:

			FORK_AND_RUN(c, pipes, pipec);

			exit(EXIT_SUCCESS);
		default:

			break;
		}
	} else {
		int j = FORK_AND_RUN(c, pipes, pipec);

		return j;
	}

	return 0;

}

void
pipe_and_execute(parsed_input *parsed, command *commands[])
{

	int status = 0;
	int wpid = 0;
	int pipec = parsed->comc - 1;
	pid_t pids[max_comms];
	int pipes[max_comms][2];
	int j = 0;
	int rtv = 0;

	for (int i = 0; i < pipec; i++) {
		if (pipe(pipes[i]) != 0) {
			fprintf(stderr, "pipe failed");
			break;
		}
	}
	for (int i = 0; i < parsed->comc; i++) {
		commands[i] = generate_command(parsed, i);
		switch (commands[i]->error) {
		case 1:
			fprintf(stderr, "Error: could not find variable\n");
			parsed->comc = -1;
			break;

		case 2:
			fprintf(stderr,
				"Error: invalid input (argument count exceeds maximum size(%d))\n",
				max_args);
			parsed->comc = -1;
			break;

		default:

			rtv =
			    execute_command(parsed->bg, commands[i], pipec,
					    pipes);

			if (rtv != 0) {
				pids[j] = rtv;
				if (pids[j] < 0) {
					parsed->comc = -1;
				} else {
					j++;
				}
			}

		}
	}

	for (int i = 0; i < pipec; i++) {
		close(pipes[i][1]);
		close(pipes[i][0]);
	}

	while ((wpid = wait(&status)) > 0) ;

}

int
main(int argc, char *argv[])
{
	char *input = malloc(max_inputsize);
	command *commands[max_comms];

	print_CWD();
	while (read_input(input) == 0) {

		parsed_input *parsed = parse_input(input);

		switch (parsed->error) {
		case 3:
			break;
		case 1:
			fprintf(stderr, "Error: invalid input\n");
			break;

		case 2:
			fprintf(stderr,
				"Error: command count exceeds maximun ammount(%d)\n",
				max_comms);
			break;

		default:
			pipe_and_execute(parsed, commands);
		}

		for (int i = 0; i < parsed->comc; i++) {
			free(commands[i]);
		}

		free(parsed);

		print_CWD();
	}

	free(input);

	exit(EXIT_SUCCESS);

}
