#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#define LSH_BUFSIZE 1024
#define LSH_TOKEN_BUFSIZE 64
#define LSH_TOKEN_DELIM " \t\r\n\a"

// Function declarations for builtin commands
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

char *builtin_commands[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};

// Read a line from stdin
char *lsh_read_line() {
    int bufsize = LSH_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "lsh: memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();

        // If EOF is reached, replace it will a null character and return
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }

        position++;

        // If we have exceeded the buffer, reallocate
        if (position >= bufsize) {
            bufsize += LSH_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "lsh: memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

// Tokenize the input from stdin
void tokenize(char *line, char **token, int startpos) {
    // Starting at startpos, find the length of the first token, using ' ' as a delimeter
    int token_len = 0, position = startpos;

    while (line[position] != ' ' && line[position] != '\0'){
        token_len++;
        position++;
    }
    *token = malloc(sizeof(char) * token_len + 1);
    if (!*token) {
        fprintf(stderr, "lsh: memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    // Copy over each char of the token from line to token
    position = 0;
    while (position < token_len) {              
        (*token)[position] = line[position];
        position++;
    }
    (*token)[position] = '\0';
}

// Parse the input from stdin, saving each command as indiviual tokens
char **lsh_parse_line(char *line) {
    int position = 0, bufsize = LSH_TOKEN_BUFSIZE;
    char **tokens = malloc(sizeof(char*) * bufsize);
    char *token;
    if (!tokens) {
        fprintf(stderr, "lsh: memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    tokenize(line, &token, position);
    while (strlen(token) > 0) {
        tokens[position] = malloc(strlen(token) + 1);
        strcpy(tokens[position], token);
        position++;

        if (position >= bufsize) {
            bufsize += LSH_TOKEN_BUFSIZE;
            tokens = realloc(tokens, sizeof(char*) * bufsize);
            if (!tokens) {
                fprintf(stderr, "lsh: memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        free(token);
        // The new startpos is the position after the previous token ended
        tokenize(line, &token, strlen(tokens[position - 1]) + 1);
    }
    tokens[position] = NULL;
    return tokens;
}

int lsh_launch(char **args) {
    __pid_t parent_id, wpid;
    int status;

    parent_id = fork();
    if (parent_id == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (parent_id < 0) {
        // Error forking
        perror("lsh");
    } else {
        // Parent process
        do {
            wpid = waitpid(parent_id, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return EXIT_FAILURE;
}

int lsh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
        }
    }

    return 1;
}

int lsh_help(char **args) {
    printf("Ethan Mason's LSH, created using a tutorial by Stephen Brenner.\n");
    printf("The following programs are built in:\n");

    for (int i = 0; i < sizeof(builtin_commands) / sizeof(char *); i++) {
        printf(" %s\n", builtin_commands[i]);
    }

    return 1;
}

int lsh_exit(char **args) {
    return 0;
}

int lsh_execute(char **args) {
    if (args[0] == NULL) {
        return 1;
    }

    for (int i = 0; i < sizeof(builtin_commands) / sizeof(char *); i++) {
        if (strcmp(args[0], builtin_commands[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return lsh_launch(args);
}


void lsh_loop() {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = lsh_read_line();
        args = lsh_parse_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);
    } while (status);
}


int main(int argc, int** argv) {
    lsh_loop();

    return EXIT_SUCCESS;
}