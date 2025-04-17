#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <string.h>

void on_child_exit() {
    waitpid(-1, NULL, 0);
    const char *logfile = "log.txt";
    FILE *file = fopen(logfile, "a");
    if (file == NULL) {
        perror("Failed to open log file");
        return;
    }
    fprintf(file, "Child terminated\n");
    fclose(file); // should i close it or keep it opened?
}

int setup_environment() {
    if (chdir("/") != 0) {
        perror("Failed to change directory");
        return 1;
    }
}

char * take_input() {
    char *input = (char *)malloc(sizeof(char) * 255);
    fgets(input, sizeof(char) * 255, stdin);
    input[strlen(input) - 1] = '\0'; // removing newline character
    return input;
}

char ** parse_input(char *input, int *size, char **command) {
    int i = 0, count = 1;
    while (input[i] != '\0') {
        // if multiple spaces after each other, if there are spaces in the end
        if (input[i] == ' ') {
            count++;
        }
        i++;
    }
    char **arg = (char **)malloc(sizeof(char *) * (count-1));
    i = 0;
    int j = 0;
    *command = input;
    j++;
    while (input[i] != '\0' && j < count) {
        if (input[i] == ' ') {
            input[i] = '\0';
            arg[j-1] = &input[i+1];
            j++;
        }
        i++;
    }
    *size = count-1;
    return arg;
}

int execute_shell_builtin(char *command, char **args, int size) {
    if (strcmp(command, "cd") == 0) {
        return 0;
    }
    else if (strcmp(command, "echo") == 0) {
        return 0;
    }
    else if (strcmp(command, "export") == 0) {
        return 0;
    } 
    return -1;
}

int execute_command(char *command, char **args, int size) {
    pid_t child_id = fork();
    if (child_id == 0) {
        // if invalid command or args it will return -1
        return execvp((command[0] == '&' ? command + 1 : command), args);
    }
    else if (child_id < 0){
        printf("couldn't intiate a process\n");
    }
    else if (command[0] != '&') {
        waitpid(child_id, NULL, 0);
    }
    return 0;
}

int shell() {
    while (1) {
        char *input = take_input();
        int size;
        char *command;
        char **args = parse_input(input, &size, &command);

        // debug
        printf("args size = %d\n", size);
        printf("command: %s\n", command);
        for (int i = 0; i < size; i++)
            printf("arg %d: %s\n", i, args[i]);

        if (strcmp(command, "exit") == 0)
            exit(0);

        int status = execute_shell_builtin(command, args, size);
        if (status == -1)
            status = execute_command(command, args, size);

        printf("status = %d\n", status);
        if (status == -1)
            printf("Invalid Command\n");
    }
}

int main() {
    signal(SIGCHLD, on_child_exit);
    setup_environment();
    shell();
}