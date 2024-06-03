#include <stdio.h>
#include <stdlib.h>

#define LSH_BUFSIZE 1024;
#define LSH_TOKEN_BUFSIZE 64;
#define LSH_TOKEN_DELIM " \t\r\n\a"

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
void tokenize(char *line, char *token, int startpos) {
    if (!line) {
        token = NULL;
        exit(EXIT_SUCCESS);
    }

    // Starting at startpos, find the length of the first token, using ' ' as a delimeter
    int token_len = 0, position = startpos;
    while (line[position] != ' ') token_len++, position++;          // && line[position] != '\0' necessary?
    token = malloc(sizeof(char) * token_len + 1);
    if (!token) {
        fprintf(stderr, "lsh: memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    // Copy over each char of the token from line to token
    position = 0;
    while (line[position] != ' ') {              // && line[position] != '\0' necessary?
        token[position] = line[position];
        position++;
    }
    token[position] = '\0';
}

// Parse the input from stdin, saving each command as indiviual tokens
char **lsh_parse_line(char *line) {
    int position = 0, bufsize = LSH_TOKEN_BUFSIZE;
    char **tokens = malloc(sizeof(char) * bufsize);
    char *token;
    if (!tokens) {
        fprintf(stderr, "lsh: memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    tokenize(line, token, position);
    while (token) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += LSH_TOKEN_BUFSIZE;
            tokens = realloc(tokens, sizeof(char) * bufsize);
            if (!tokens) {
                fprintf(stderr, "lsh: memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        free(token);
        // The new startpos is the position after the previous token ended
        tokenize(line, token, sizeof(token[position - 1]));
    }
    tokens[position] = NULL;
    return tokens;
}

int lsh_launch(char **args) {
    return EXIT_SUCCESS;
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