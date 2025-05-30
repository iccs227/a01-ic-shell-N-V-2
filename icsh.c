/* ICCS227: Project 1: icsh
  Name: Nuttapanee Visitatimat
  StudentID: 6580975
*/

#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#define MAX_CMD_BUFFER 255

static pid_t foreground_pid = 0;
int last_exit_status = 0;
int last_signal = 0; 

void set_pid_foreground(pid_t pid) {
    foreground_pid = pid;
}

void handle_signal(int sig) {
    if (foreground_pid > 0) {
        kill(-foreground_pid, sig);
        last_signal = sig;
    }
}

void signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
    signal(SIGTTOU, SIG_IGN);
}

int external_program(char buffer[]) {
    int status;
    last_signal = 0;
    size_t len = strlen(buffer);
    while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == ' ')) {
        buffer[len - 1] = '\0';
        len--;
    }
    char* p = strtok(buffer, " ");
    char* command[MAX_CMD_BUFFER / 2 + 1];
    int i = 0;
    while (p != NULL && i < MAX_CMD_BUFFER / 2) { 
        command[i++] = p;
        p = strtok(NULL, " ");
    }
    command[i] = NULL;
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return 0;
    }
    if (pid == 0) {
        setpgid(0, 0);
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        execvp(command[0], command);
        perror("execvp failed");
        exit(1);
    } else {
        setpgid(pid, pid);
        signal(SIGTTOU, SIG_IGN);
        tcsetpgrp(STDIN_FILENO, pid);
        set_pid_foreground(pid);
        while (1) {
            if (waitpid(pid, &status, WUNTRACED) == -1) {
                perror("waitpid");
                break;
            }
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                last_exit_status = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
                break;
            }
            if (WIFSTOPPED(status)) {
                printf("\n[%d] Stopped\n", pid);
                last_exit_status = 128 + WSTOPSIG(status);
                break;
            }
        }
        tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
        signal(SIGTTOU, SIG_DFL);
        set_pid_foreground(0);
        return last_exit_status;   
    }
}

int interactive_shell() { 
    char buffer[MAX_CMD_BUFFER];
    char previous_buffer[MAX_CMD_BUFFER] = "";
    printf("Starting IC shell \n");
    printf("Welcome to my shell ~~~  ૮ ˶ᵔ ᵕ ᵔ˶ ა ~~~ \n");
    printf("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣤⡤⠤⠤⠤⣤⣄⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
           "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡤⠞⠋⠁⠀⠀⠀⠀⠀⠀⠀⠉⠛⢦⣤⠶⠦⣤⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
           "⠀⠀⠀⠀⠀⠀⠀⢀⣴⠞⢋⡽⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠃⠀⠀⠙⢶⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
           "⠀⠀⠀⠀⠀⠀⣰⠟⠁⠀⠘⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⡀⠀⠀⠉⠓⠦⣤⣤⣤⣤⣤⣤⣄⣀⠀⠀⠀\n"
           "⠀⠀⠀⠀⣠⠞⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣴⣷⡄⠀⠀⢻⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠻⣆⠀\n"
           "⠀⠀⣠⠞⠁⠀⠀⣀⣠⣏⡀⠀⢠⣶⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⠿⡃⠀⠀⠀⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⡆\n"
           "⢀⡞⠁⠀⣠⠶⠛⠉⠉⠉⠙⢦⡸⣿⡿⠀⠀⠀⡄⢀⣀⣀⡶⠀⠀⠀⢀⡄⣀⠀⣢⠟⢦⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⠃\n"
           "⡞⠀⠀⠸⠁⠀⠀⠀⠀⠀⠀⠀⢳⢀⣠⠀⠀⠀⠉⠉⠀⠀⣀⠀⠀⠀⢀⣠⡴⠞⠁⠀⠀⠈⠓⠦⣄⣀⠀⠀⠀⠀⣀⣤⠞⠁⠀\n"
           "⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⠀⠁⠀⢀⣀⣀⡴⠋⢻⡉⠙⠾⡟⢿⣅⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠉⠙⠛⠉⠉⠀⠀⠀⠀\n"
           "⠘⣦⡀⠀⠀⠀⠀⠀⠀⣀⣤⠞⢉⣹⣯⣍⣿⠉⠟⠀⠀⣸⠳⣄⡀⠀⠀⠙⢧⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
           "⠀⠈⠙⠒⠒⠒⠒⠚⠋⠁⠀⡴⠋⢀⡀⢠⡇⠀⠀⠀⠀⠃⠀⠀⠀⠀⠀⢀⡾⠋⢻⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
           "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡇⠀⢸⡀⠸⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠀⠀⢠⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
           "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⣇⠀⠀⠉⠋⠻⣄⠀⠀⠀⠀⠀⣀⣠⣴⠞⠋⠳⠶⠞⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
           "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠳⠦⢤⠤⠶⠋⠙⠳⣆⣀⣈⡿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
           "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
    signal_handlers();
    while (1) {
        printf("Wah  ࣪𖤐. ");
        fgets(buffer, 255, stdin);
        if (strcmp(buffer, "") == 0) {
            continue;
        }
        if (strncmp("!!", buffer, 2) == 0) {
            if (strlen(previous_buffer) == 0) {
                printf("No previous command.\n");
                continue;
            }
            printf("%s", previous_buffer);
            strncpy(buffer, previous_buffer, MAX_CMD_BUFFER - 1);
            buffer[MAX_CMD_BUFFER - 1] = '\0';
        }
        if (strncmp(buffer, "echo",4) == 0) {
            if (strncmp("echo $?", buffer, 7) == 0) {
                printf("%d\n", last_exit_status);
            } else {
                int length = strlen(buffer) - 5;
                char text[length + 1];
                strncpy(text, buffer + 5, length);
                text[length] = '\0';
                printf("%s", text);
            }
        } else if (strncmp("exit ", buffer, 4) == 0) {
            int length = strlen(buffer) - 5;
            char text[length];
            strncpy(text, buffer + 5, length);
            text[length] = '\0';
            int num = 0;
            if (strncmp("exit ", buffer, 5) == 0) {
                num = atoi(text) & 0xFF;
            }
            printf("Bye ~~~~\n");
            printf("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢎⠱⠊⡱⠀⠀⠀⠀⠀⠀\n"
                   "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡠⠤⠒⠒⠒⠒⠤⢄⣑⠁⠀⠀⠀⠀⠀⠀⠀⠀\n"
                   "⠀⠀⠀⠀⠀⠀⠀⢀⡤⠒⠝⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠲⢄⡀⠀⠀⠀⠀⠀\n"
                   "⠀⠀⠀⠀⠀⢀⡴⠋⠀⠀⠀⠀⣀⠀⠀⠀⠀⠀⠀⢠⣢⠐⡄⠀⠉⠑⠒⠒⠒⣄\n"
                   "⠀⠀⠀⣀⠴⠋⠀⠀⠀⡎⢀⣘⠿⠀⠀⢠⣀⢄⡦⠀⣛⣐⢸⠀⠀⠀⠀⠀⠀⢘\n"
                   "⡠⠒⠉⠀⠀⠀⠀⠀⡰⢅⠣⠤⠘⠀⠀⠀⠀⠀⠀⢀⣀⣤⡋⠙⠢⢄⣀⣀⡠⠊\n"
                   "⢇⠀⠀⠀⠀⠀⢀⠜⠁⠀⠉⡕⠒⠒⠒⠒⠒⠛⠉⠹⡄⣀⠘⡄⠀⠀⠀⠀⠀⠀\n"
                   "⠀⠑⠂⠤⠔⠒⠁⠀⠀⡎⠱⡃⠀⠀⡄⠀⠄⠀⠀⠠⠟⠉⡷⠁⠀⠀⠀⠀⠀⠀\n"
                   "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⠤⠤⠴⣄⡸⠤⣄⠴⠤⠴⠄⠼⠀⠀⠀⠀⠀⠀⠀⠀\n");
            return num;
        } else {
            char buffer_copy[MAX_CMD_BUFFER];
            strcpy(buffer_copy, buffer);
            strcat(buffer_copy, "\n");
            if (!external_program(buffer_copy)) {
                if (last_signal != 0) {
                    printf("Bad Command!! \n");
                }
            }
        }
        if (strcmp(buffer, "!!") != 0) {
            strncpy(previous_buffer, buffer, MAX_CMD_BUFFER - 1);
            previous_buffer[MAX_CMD_BUFFER - 1] = '\0';
        }
    }
}

int no_interactive_shell(char buffer[], char previous_buffer[]) {
    if (strcmp(buffer, "\n") == 0) {
        return 0;
    }
    if (strncmp(buffer, "#", 1) == 0) {
        return 0;
    }
    if (strncmp("!!", buffer, 2) == 0) {
        if (strlen(previous_buffer) == 0) {
            printf("No previous command.\n");
            return 0;
        }
        no_interactive_shell(previous_buffer, buffer);
        return 0;
    }
    if (strncmp("echo ", buffer, 5) == 0) {
        printf("%s", buffer + 5);
        return 0;
    }
    if (strncmp("exit ", buffer, 4) == 0) {
        if (strncmp("exit ", buffer, 5) == 0) {
            int num = atoi(buffer + 5) & 0xFF;
            return num;
        }
        return 0;
    }
    char buffer_copy[MAX_CMD_BUFFER];
    strcpy(buffer_copy, buffer);
    strcat(buffer_copy, "\n");  
    if (external_program(buffer_copy)) {
        return 0;  
    } else {
        printf("Bad Command!!\n");
        return 0;
    }
}

int main(int argc, char *argv[]) {
    int exit_num;
    if (argc >= 2) {
        FILE *file;
        char buffer[MAX_CMD_BUFFER];
        char previous_buffer[MAX_CMD_BUFFER];
        file = fopen(argv[1], "r");
        while (fgets(buffer, MAX_CMD_BUFFER, file)) {
            exit_num = no_interactive_shell(buffer, previous_buffer);
            strncpy(previous_buffer, buffer, MAX_CMD_BUFFER - 1);
        }
        fclose(file);
    } else {
        exit_num = interactive_shell();
    }
    return exit_num;
}

