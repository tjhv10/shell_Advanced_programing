#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <termios.h>

#define OUT 0
#define APP 1
#define IN 2
#define COMMAND_SIZE 1024
#define LINE_COMMAND_SIZE 50

#define TRUE 1

#define END_L_STR "\0"
#define END_L_CHR '\0'

#define QUIT "quit"
#define HELLO "hello"
#define CONTROL_C "You typed Control-C!\n"

#define PIPE_STR "|"
#define PIPE_CHR '|'
#define EMPTY_STRING " "
#define EMPTY_CHAR ' '
#define AGAIN "!!"
#define AND '&'
#define STDOUT_CHR '>'
#define STDIN_CHR '<'
#define APPEND_STR ">>"

#define CD_STR "cd"
#define PROMPT_STR "prompt"
#define ECHO_STR "echo"
#define READ_STR "read"
#define PROMPT_STR_AFTER ": "

#define HISTORY_SIZE 100

int status;
char *prompt = HELLO;
char *command_history[HISTORY_SIZE];
int history_count = 0;
int history_index = -1;

void c_handler(int);
void pipe_tasks(char *);
void async_tasks(char *);
void redirect_tasks(char *, int);
void other_tasks(char *);
void set_variable(char *, char *);
char *get_variable(char *);
void parse_if_statement(char *);
int parse_command(char **parsed_command, char *cmd, const char *delimiter);

void add_to_history(char *command) {
    if (history_count < HISTORY_SIZE) {
        command_history[history_count++] = strdup(command);
    } else {
        free(command_history[0]);
        for (int i = 1; i < HISTORY_SIZE; ++i) {
            command_history[i - 1] = command_history[i];
        }
        command_history[HISTORY_SIZE - 1] = strdup(command);
    }
}

char *get_previous_command() {
    if (history_index > 0) {
        return command_history[--history_index];
    } else if (history_index == 0) {
        return command_history[history_index];
    }
    return "";
}

char *get_next_command() {
    if (history_index < history_count - 1) {
        return command_history[++history_index];
    }
    return "";
}

int parse_command(char **parsed_command, char *cmd, const char *delimiter) {
    char *cmd_copy = strdup(cmd);
    if (!cmd_copy) {
        perror("strdup");
        return -1;
    }

    char *token;
    token = strtok(cmd_copy, delimiter);
    int counter = -1;

    while (token) {
        parsed_command[++counter] = malloc(strlen(token) + 1);
        if (!parsed_command[counter]) {
            perror("malloc");
            free(cmd_copy);
            return -1;
        }
        strcpy(parsed_command[counter], token);

        if (strcmp(delimiter, PIPE_STR) == 0) {
            if (parsed_command[counter][strlen(token) - 1] == EMPTY_CHAR) {
                parsed_command[counter][strlen(token) - 1] = END_L_CHR;
            }
            if (parsed_command[counter][0] == EMPTY_CHAR) {
                memmove(parsed_command[counter], parsed_command[counter] + 1, strlen(token));
            }
        }
        parsed_command[counter][strlen(token) + 1] = END_L_CHR;
        token = strtok(NULL, delimiter);
    }
    parsed_command[++counter] = NULL;

    free(cmd_copy);
    return counter;
}

void c_handler(int sig) {
    char *msg = CONTROL_C;
    char final_msg[LINE_COMMAND_SIZE];
    strcpy(final_msg, msg);
    strcat(final_msg, prompt);
    strcat(final_msg, PROMPT_STR_AFTER);
    strcat(final_msg, END_L_STR);
    write(1, final_msg, strlen(final_msg));
}

void pipe_tasks(char *cmd) {
    char *parsed_command[LINE_COMMAND_SIZE];
    int commands = parse_command(parsed_command, cmd, PIPE_STR);
    char *inner_cmd[LINE_COMMAND_SIZE];
    int fd[2];
    pid_t pid;

    for (int i = 0; i < commands; ++i) {
        parse_command(inner_cmd, parsed_command[i], EMPTY_STRING);
        if (i != commands - 1) {
            if (pipe(fd) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            if (i != 0) {
                dup2(fd[0], 0);
                close(fd[0]);
            }
            if (i != commands - 1) {
                dup2(fd[1], 1);
                close(fd[1]);
            }
            execvp(inner_cmd[0], inner_cmd);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
            if (i != commands - 1) {
                close(fd[1]);
                fd[0] = fd[0];
            }
        }
    }
}

void async_tasks(char *cmd) {
    if (fork() == 0) {
        char *parsed_command[LINE_COMMAND_SIZE];
        int commands = parse_command(parsed_command, cmd, EMPTY_STRING);
        parsed_command[commands - 1] = NULL;
        execvp(parsed_command[0], parsed_command);
    }
}

void redirect_tasks(char *command, int direction) {
    if (fork() == 0) {
        char *parsed_command[LINE_COMMAND_SIZE];
        int commands = parse_command(parsed_command, command, EMPTY_STRING);
        int fd;
        switch (direction) {
            case OUT:
                fd = creat(parsed_command[commands - 1], 0660);
                dup2(fd, 1);
                break;
            case APP:
                fd = open(parsed_command[commands - 1], O_CREAT | O_APPEND | O_RDWR, 0660);
                dup2(fd, 1);
                break;
            case IN:
                fd = open(parsed_command[commands - 1], O_RDONLY, 0660);
                dup2(fd, 0);
                break;
            default:
                break;
        }
        parsed_command[commands - 2] = parsed_command[commands - 1] = NULL;
        execvp(parsed_command[0], parsed_command);
    } else {
        wait(&status);
    }
}

void other_tasks(char *command) {
    char *parsed_command[LINE_COMMAND_SIZE];
    parse_command(parsed_command, command, EMPTY_STRING);
    if (!strcmp(parsed_command[0], CD_STR)) {
        chdir(parsed_command[1]);
    } else if (!strcmp(parsed_command[0], PROMPT_STR)) {
        prompt = parsed_command[2];
    } else if (!strcmp(parsed_command[0], ECHO_STR) && !strcmp(parsed_command[1], "$?")) {
        printf("%d\n", status);
    } else if (!strcmp(parsed_command[0], ECHO_STR) && parsed_command[1][0] == '$') {
        char *variable_name = parsed_command[1] + 1;
        variable_name[strlen(variable_name)] = END_L_CHR;
        char *result = get_variable(variable_name);
        printf("%s\n", result);
    } else if (!strcmp(parsed_command[0], READ_STR)) {
        char buffer[COMMAND_SIZE];
        fgets(buffer, COMMAND_SIZE, stdin);
        buffer[strlen(buffer) - 1] = END_L_CHR;
        set_variable(parsed_command[1], buffer);
    } else if (parsed_command[0][0] == '$' && parsed_command[1][0] == '=') {
        char *variable_name = parsed_command[0] + 1;
        variable_name[strlen(variable_name)] = END_L_CHR;
        set_variable(variable_name, parsed_command[2]);
    } else if (!strcmp(parsed_command[0], "if")) {
        parse_if_statement(command);
    } else if (fork() == 0) {
        execvp(parsed_command[0], parsed_command);
    } else {
        wait(&status);
    }
}

void parse_if_statement(char *if_command) {
    char *parsed_command[LINE_COMMAND_SIZE];
    parse_command(parsed_command, if_command, EMPTY_STRING);
    char condition[COMMAND_SIZE] = "";
    char then_block[COMMAND_SIZE] = "";
    char else_block[COMMAND_SIZE] = "";
    int in_then = 0, in_else = 0;

    for (int i = 1; parsed_command[i] != NULL; i++) {
        strcat(condition, parsed_command[i]);
        strcat(condition, " ");
    }

    char line[COMMAND_SIZE];
    while (TRUE) {
        printf("> ");
        fgets(line, COMMAND_SIZE, stdin);
        line[strlen(line) - 1] = END_L_CHR;

        if (strcmp(line, "then") == 0) {
            in_then = 1;
        } else if (strcmp(line, "else") == 0) {
            in_then = 0;
            in_else = 1;
        } else if (strcmp(line, "fi") == 0) {
            break;
        } else {
            if (in_then) {
                strcat(then_block, line);
                strcat(then_block, "\n");
            } else if (in_else) {
                strcat(else_block, line);
                strcat(else_block, "\n");
            }
        }
    }

    if (system(condition) == 0) {
        if (strchr(then_block, PIPE_CHR)) {
            pipe_tasks(then_block);
        } else {
            other_tasks(then_block);
        }
    } else {
        if (strchr(else_block, PIPE_CHR)) {
            pipe_tasks(else_block);
        } else {
            other_tasks(else_block);
        }
    }
}

void set_variable(char *name, char *value) {
    int sv = setenv(name, value, 1);
    if (sv) {
        perror("Error set env");
    }
}

char *get_variable(char *name) {
    return getenv(name);
}

void clear_line() {
    printf("\033[2K\033[1G"); // Clear line and move cursor to the beginning
    printf("%s: ", prompt);
    fflush(stdout);
}

int main() {
    struct termios orig_termios, new_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);
    new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    signal(SIGINT, c_handler);
    char command[COMMAND_SIZE], saved_cmd[COMMAND_SIZE];

    while (TRUE) {
        printf("%s: ", prompt);
        fflush(stdout);

        int index = 0;
        char ch;
        while (read(STDIN_FILENO, &ch, 1) == 1) {
            if (ch == '\n') {
                command[index] = END_L_CHR;
                printf("\n");
                break;
            } else if (ch == 27) { // Arrow keys
                char seq[3];
                if (read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1) {
                    if (seq[0] == '[') {
                        if (seq[1] == 'A') {
                            strcpy(command, get_previous_command());
                            index = strlen(command);
                            clear_line();
                            write(STDOUT_FILENO, &command, index);
                        } else if (seq[1] == 'B') {
                            strcpy(command, get_next_command());
                            index = strlen(command);
                            clear_line();
                            write(STDOUT_FILENO, &command, index);
                        }
                    }
                }
            } else if (ch == 127 || ch == '\b') { // Backspace key
                if (index > 0) {
                    index--;
                    command[index] = '\0';
                    clear_line();
                    write(STDOUT_FILENO, &command, index);
                }
            } else {
                command[index++] = ch;
                write(STDOUT_FILENO, &ch, 1);
            }
        }

        if (index == 0) {
            continue;
        }

        add_to_history(command);
        history_index = history_count;

        if (!strcmp(command, QUIT)) {
            break;
        } else if (!strcmp(command, AGAIN)) {
            strcpy(command, saved_cmd);
        } else {
            strcpy(saved_cmd, command);
        }

        if (strchr(command, PIPE_CHR) && !strstr(command, "if")) {
            pipe_tasks(command);
        } else if (strchr(command, AND)) {
            async_tasks(command);
        } else if (strchr(command, STDOUT_CHR) && !strstr(command, APPEND_STR)) {
            redirect_tasks(command, OUT);
        } else if (strchr(command, STDIN_CHR)) {
            redirect_tasks(command, IN);
        } else if (strstr(command, APPEND_STR)) {
            redirect_tasks(command, APP);
        } else {
            other_tasks(command);
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    for (int i = 0; i < history_count; ++i) {
        free(command_history[i]);
    }

    return 0;
}
