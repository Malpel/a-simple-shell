#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define SHELL_RL_BUFSIZE 1024
#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"

int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);


char *builtin_str[] = {
	"cd",
	"help",
	"exit"
};


int (*builtin_func[]) (char **) = {
	&shell_cd,
	&shell_help,
	&shell_exit
};


int shell_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}


int shell_cd(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "shell: expecteded argument to \"cd\"\n");
	} else {
		if (chdir(args[1]) != 0) {
			perror("shell");
		}
	}
	return 1;
}


int shell_help(char **args)
{
	int i;
	printf("A Simple Shell\n");
	printf("functions built in: \n");

	for (i = 0; i < shell_num_builtins(); i++) {
		printf("  %s\n", builtin_str[i]);
	}
	return 1;
}


int shell_exit(char **args)
{
	return 0;
}	


int shell_launch(char **args)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		//child process
		if (execvp(args[0], args) == -1) {
			perror("shell");
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		// error forking
		perror("shell");
	} else {
		// parent process
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}


int shell_execute(char **args)
{
	int i;

	if (args[0] == NULL) {
		return 1;
	}

	for (i = 0; i < shell_num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}
	return shell_launch(args);
}





char *shell_read_line(void)
{
	int bufsize = SHELL_RL_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;

	if (!buffer) {
		fprintf(stderr, "shell: allocation error\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		//read character
		c = getchar();

		// if end of file, replace with null character and return
		if (c == EOF) {
			exit(EXIT_SUCCESS);
		} else if (c == '\n') {
			buffer[position] = '\0';
			return buffer;
		} else {
			buffer[position] = c;
		}
		position++;

		// if buffer is exceeded, reallocate
		if (position >= bufsize) {
			bufsize += SHELL_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if (!buffer) {
				fprintf(stderr, "shell: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}



char **shell_split_line(char *line)
{
	int bufsize = SHELL_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token, **tokens_backup;

	if (!tokens) {
		fprintf(stderr, "shell:allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, SHELL_TOK_DELIM);
	while (token != NULL) {
		tokens[position] = token;
		position++;

		if (position >= bufsize) {
			bufsize += SHELL_TOK_BUFSIZE;
			tokens_backup = tokens;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				free(tokens_backup);
				fprintf(stderr, "shell: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, SHELL_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}


void shell_loop(void)
{
	char *line;
	char **args;
	int status;

	do {
		printf("Shell> ");
		line = shell_read_line();
		args = shell_split_line(line);
		status = shell_execute(args);

		free(line);
		free(args);
	} while (status);
}


int main(int argc, char **argv)
{
	shell_loop();

	return EXIT_SUCCESS;
}


