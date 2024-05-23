#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>

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

int status;
char *prompt = HELLO;

void c_handler(int);
void pipe_tasks(char *);
void async_tasks(char *);
void redirect_tasks(char *, int);
void other_tasks(char *);
void set_variable(char *, char *);
char * get_variable(char *);

int parse_command(char **parsed_command, char *cmd, const char * delimeter) {
    char * token;
    token = strtok(cmd, delimeter);
    int counter = -1;

    while(token) {
        parsed_command[++counter] = malloc(strlen(token) + 1);
        strcpy(parsed_command[counter], token);
        if (delimeter==PIPE_STR) {
            if (parsed_command[counter][strlen(token) - 1] == EMPTY_CHAR) {
                parsed_command[counter][strlen(token) - 1] = END_L_CHR;
            }
            if (parsed_command[counter][0] == EMPTY_CHAR) {
                memmove(parsed_command[counter], parsed_command[counter] + 1, strlen(token));
            }
        }
        parsed_command[counter][strlen(token) + 1] = END_L_CHR;
        token = strtok(NULL, delimeter);
    }
    parsed_command[++counter] = NULL;
    return counter;
}

void c_handler(int sig) {
    char * msg = CONTROL_C;
    char final_msg[LINE_COMMAND_SIZE];
    strcpy(final_msg, msg);
    strcat(final_msg, prompt);
    strcat(final_msg, PROMPT_STR_AFTER);
    strcat(final_msg, END_L_STR);
    write(1, final_msg, strlen(final_msg));
}

void pipe_tasks(char *cmd)
{
    char *parsed_command[LINE_COMMAND_SIZE];
    int commands = parse_command(parsed_command, cmd, PIPE_STR);
    char *inner_cmd[LINE_COMMAND_SIZE];
    int fd[commands][2];
    for (int i = 0; i < commands; ++i) {
        parse_command(inner_cmd, parsed_command[i], EMPTY_STRING);
        if (i != commands - 1)
            pipe(fd[i]);

        if (fork() == 0) {
            if (i != commands - 1) {
                dup2(fd[i][1], 1);
                close(fd[i][0]);
                close(fd[i][1]);
            }
            if (i != 0) { // not parent
                dup2(fd[i - 1][0], 0);
                close(fd[i - 1][0]);
                close(fd[i - 1][1]);
            }
            execvp(inner_cmd[0], inner_cmd); // execute
        }
        if (i != 0) {
            close(fd[i - 1][0]);
            close(fd[i - 1][1]);
        }
        wait(NULL); // wait for next command
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

void redirect_tasks(char * command, int direction) {
    if (fork() == 0) { // child
        char * parsed_command[LINE_COMMAND_SIZE];
        int commands = parse_command(parsed_command, command, EMPTY_STRING);
        int fd;
        switch (direction) {
        case OUT: // output
            fd = creat(parsed_command[commands - 1], 0660);
            dup2(fd, 1);
            break;

        case APP: // append
            fd = open(parsed_command[commands - 1], O_CREAT | O_APPEND | O_RDWR, 0660);
            dup2(fd, 1);
            break;

        case IN: // input
            fd = open(parsed_command[commands - 1], O_RDONLY, 0660);
            dup2(fd, 0);
            break;

        default:
            break;
        }

        parsed_command[commands - 2] = parsed_command[commands - 1] = NULL;
        execvp(parsed_command[0], parsed_command);
    } else {
        wait(&status); // wait for child to finish.
    }
}

void other_tasks(char *command) {
    char *parsed_command[LINE_COMMAND_SIZE];
    parse_command(parsed_command, command, EMPTY_STRING);

    if (!strcmp(parsed_command[0], CD_STR)) {
        chdir(parsed_command[1]);
    }

    else if (!strcmp(parsed_command[0], PROMPT_STR)) {
        prompt = parsed_command[2];
    }

    else if (!strcmp(parsed_command[0], ECHO_STR) && !strcmp(parsed_command[1], "$?")) {
        printf("%d\n", status);
    }

    else if (!strcmp(parsed_command[0], ECHO_STR) && parsed_command[1][0] == '$') {
        char *variable_name = parsed_command[1] + 1;
        variable_name[strlen(variable_name)] = END_L_CHR;
        char *result = get_variable(variable_name);
        printf("%s\n", result);
    }

    else if (!strcmp(parsed_command[0], READ_STR)) {
        char buffer[COMMAND_SIZE];
        fgets(buffer, COMMAND_SIZE, stdin);
        buffer[strlen(buffer) - 1] = END_L_CHR;
        set_variable(parsed_command[1], buffer);
    }

    else if (parsed_command[0][0] == '$' && parsed_command[1][0] == '=') {
        char *variable_name = parsed_command[0] + 1;
        variable_name[strlen(variable_name)] = END_L_CHR;
        set_variable(variable_name, parsed_command[2]);
    }

    else if (!strcmp(parsed_command[0], "if")) {
        // Check if the condition is true
        if (strcmp(parsed_command[1], "|")) { // Just a placeholder condition
            // Execute the commands between "then" and "else"
            int i = 2;
            while (strcmp(parsed_command[i], "then")) {
                other_tasks(parsed_command[i]);
                i++;
            }
            // Skip the "then" and execute the commands until "else" or "fi"
            i++;
            while (strcmp(parsed_command[i], "else") && strcmp(parsed_command[i], "fi")) {
                other_tasks(parsed_command[i]);
                i++;
            }
            // Skip the "else" and execute the commands until "fi"
            if (!strcmp(parsed_command[i], "else")) {
                i++;
                while (strcmp(parsed_command[i], "fi")) {
                    i++;
                }
            }
        } else {
            printf("Placeholder condition: Always true\n");
        }
    }

    else if (fork() == 0) {
        execvp(parsed_command[0], parsed_command);
    }

    else {
        wait(&status);
    }
}


void set_variable(char * name, char * value) {
    int sv = setenv(name, value, 1);
    if(sv) { // error set variable
        perror("Error set env");
    }
}

char * get_variable(char * name) {
    return getenv(name);
}

int main() {
    // handler for Ctrl+C
    signal(SIGINT, c_handler);
    char command[COMMAND_SIZE], saved_cmd[COMMAND_SIZE];

    while(TRUE) {
        printf("%s: ", prompt);
        fgets(command, COMMAND_SIZE, stdin);
        command[strlen(command) - 1] = END_L_CHR;

        if (!strcmp(command, QUIT)) {  // quit
            break;
        }

        else if(!strcmp(command, AGAIN)) {
            strcpy(command, saved_cmd);
        }

        else {
            strcpy(saved_cmd, command);
        }

        if (strchr(command, PIPE_CHR)) {
            pipe_tasks(command);
        }

        else if (strchr(command, AND)) {
            async_tasks(command);
        }

        else if (strchr(command, STDOUT_CHR) && !strstr(command, APPEND_STR)) {
            redirect_tasks(command, OUT);
        }

        else if (strchr(command, STDIN_CHR)) {
            redirect_tasks(command, IN);
        }

        else if (strstr(command, APPEND_STR)) {
            redirect_tasks(command, APP);
        }

        else {
            other_tasks(command);
        }
    }
}