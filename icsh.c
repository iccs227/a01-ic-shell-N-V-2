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
#include <fcntl.h>

#define MAX_CMD_BUFFER 255
#define MAX_JOBS 100

// -------- Set Up ------------//
typedef enum { JOB_RUNNING, JOB_STOPPED, JOB_DONE } job_status_t;

typedef struct {
    int job_id;
    pid_t pgid;
    char command[MAX_CMD_BUFFER];
    job_status_t status;
} job_t;

job_t jobs[MAX_JOBS];
int next_job_id = 1;

static pid_t foreground_pid = 0;
int last_exit_status = 0;

// -------- Signal Handling ------------//
void set_pid_foreground(pid_t pid) { // update foreground PID
    foreground_pid = pid;
}

void handle_signal(int sig) { // if SIGINT or SIGIST is pressed, pass signal to foreground job
    if (foreground_pid > 0) {
        kill(-foreground_pid, sig);
    }
}

void signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
    signal(SIGTTOU, SIG_IGN); // ignore background terminal error
} 
// -------- Job Functions---------//
void init_jobs() { // clear job
    for (int i = 0; i < MAX_JOBS; i++) {
        jobs[i].job_id = 0;
        jobs[i].status = JOB_DONE;
    }
}

int add_job(pid_t pgid, const char *command, job_status_t status) { //add job to the list
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].status == JOB_DONE) {
            jobs[i].job_id = next_job_id++;
            jobs[i].pgid = pgid;
            strncpy(jobs[i].command, command, MAX_CMD_BUFFER - 1);
            jobs[i].status = status;
            return jobs[i].job_id;
        }
    }
    return -1;
}

void remove_job(int job_id) { // mark job as done
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].job_id == job_id) {
            jobs[i].status = JOB_DONE;
            return;
        }
    }
}

void update_job_status(pid_t pgid, job_status_t status) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pgid == pgid && jobs[i].status != JOB_DONE) {
            jobs[i].status = status;
            return;
        }
    }
}

job_t *find_job(int job_id) { // check which jobs finished or stopped and print status
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].job_id == job_id && jobs[i].status != JOB_DONE) {
            return &jobs[i];
        }
    }
    return NULL;
}

void check_jobs() { 
    int status; 
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (jobs[i].pgid == pid && jobs[i].status != JOB_DONE) {
                if (WIFEXITED(status) || WIFSIGNALED(status)) {
                    printf("[%d] Done\t%s\n", jobs[i].job_id, jobs[i].command);
                    jobs[i].status = JOB_DONE;
                } else if (WIFSTOPPED(status)) {
                    jobs[i].status = JOB_STOPPED;
                    printf("[%d] Stopped\t%s\n", jobs[i].job_id, jobs[i].command);
                }
                break;
            }
        }
    }
}

void jobs_command() { // show all current jobs
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].status != JOB_DONE) {
            const char *status_str = jobs[i].status == JOB_RUNNING ? "Running" : "Stopped";
            printf("[%d] %s\t%s &\n", jobs[i].job_id, status_str, jobs[i].command);
        }
    }
}

int fg_command(char *arg) {
    if (arg == NULL || arg[0] != '%') {
        printf("Usage: fg %%<job_id>\n");
        return -1;
    }

    int job_id = atoi(arg + 1);
    job_t *job = find_job(job_id);
    if (!job) {
        printf("Job not found\n");
        return -1;
    }

    if (job->status == JOB_STOPPED) {
        kill(-job->pgid, SIGCONT);
        update_job_status(job->pgid, JOB_RUNNING);
    }
    // make job foreground
    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(STDIN_FILENO, job->pgid);
    set_pid_foreground(job->pgid);

    int status;
    while (1) {
        if (waitpid(job->pgid, &status, WUNTRACED) == -1) {
            perror("waitpid");
            break;
        }
        
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            last_exit_status = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
            remove_job(job_id);
            printf("\n");
            break;
        }
        // ctrl+Z
        if (WIFSTOPPED(status)) {
            update_job_status(job->pgid, JOB_STOPPED);
            printf("\n");
            printf("[%d]  Stopped\t%s\n", job->job_id, job->command);
            last_exit_status = 128 + WSTOPSIG(status);
            break;
        }
    }
    // return to shell
    tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
    signal(SIGTTOU, SIG_DFL);
    set_pid_foreground(0);
    return 0;
}

int bg_command(char *arg) {
    if (arg == NULL || arg[0] != '%') {
        printf("Usage: bg %%<job_id>\n");
        return -1;
    }

    int job_id = atoi(arg + 1);
    job_t *job = find_job(job_id);
    if (!job) {
        printf("Job not found\n");
        return -1;
    }

    if (job->status == JOB_STOPPED) {
        kill(-job->pgid, SIGCONT); // resume in background
        update_job_status(job->pgid, JOB_RUNNING);
        printf("[%d] %s\n", job_id, job->command);
    }
    return 0;
}

// -------- I/O redirection ---------//
int is_echo_output(const char *buffer) { // echo ... > ....  
    if (strncmp(buffer, "echo", 4) != 0) return 0;

    int gt_count = 0;
    for (int i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] == '>') gt_count++;
    }

    return gt_count == 1; // if it is true then we use redirect o/w it is a part for echo
}
// -------- External command ---------//
int external_program(char buffer[], int background) { // background = 1 if run with &, 0 if foreground

    size_t len = strlen(buffer);
    while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == ' ')) {
        buffer[len - 1] = '\0';
        len--;
    }
    // save original
    char raw_command[MAX_CMD_BUFFER];
    strncpy(raw_command, buffer, MAX_CMD_BUFFER - 1);
    raw_command[MAX_CMD_BUFFER - 1] = '\0';
    
    // check for I/O redirection
    char *infile = NULL, *outfile = NULL;
    int redirect_in = 0, redirect_out = 0;
    // >
    char *out_pos = strchr(buffer, '>');
    if (out_pos) {
        *out_pos = '\0';
        outfile = strtok(out_pos + 1, " ");
        redirect_out = 1;
    }
    // <
    char *in_pos = strchr(buffer, '<');
    if (in_pos) {
        *in_pos = '\0';
        infile = strtok(in_pos + 1, " ");
        redirect_in = 1;
    }
    // break command in to words
    char *p = strtok(buffer, " ");
    char *command[MAX_CMD_BUFFER / 2 + 1];
    int i = 0;
    while (p != NULL && i < MAX_CMD_BUFFER / 2) {
        command[i++] = p;
        p = strtok(NULL, " ");
    }
    command[i] = NULL;
    // create new process
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return 0;
    }
    // child process
    if (pid == 0) {
        setpgid(0, 0); // set process group
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        // handle input redirection
        if (redirect_in) {
            int fd_in = open(infile, O_RDONLY);
            if (fd_in < 0) {
                perror("Input file open failed");
                exit(1);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        // handle output redirection
        if (redirect_out) {
            int fd_out = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, 0666);
            if (fd_out < 0) {
                perror("Output file open failed");
                exit(1);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        // other external commands
        execvp(command[0], command);
        printf("Bad Command !!\n");
        exit(1);
    // parent process
    } else {
        setpgid(pid, pid); // assign process group
       
        if (background) {
            int job_id = add_job(pid, raw_command, background ? JOB_RUNNING : JOB_RUNNING);
            printf("[%d] %d\n", job_id, pid);
            return 0;  
        } else {
            // foreground jobs: wait until finished or stopped
            signal(SIGTTOU, SIG_IGN);
            tcsetpgrp(STDIN_FILENO, pid);
            set_pid_foreground(pid);
            
            int status;
            while (1) {
                if (waitpid(pid, &status, WUNTRACED) == -1) { 
                    perror("waitpid");
                    break;
                }
            
                if (WIFEXITED(status) || WIFSIGNALED(status)) { // case job exits normally or is killed
                    if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT) {
                        printf("\n");
                    }                    
                    last_exit_status = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
                    break;
                }
                if (WIFSTOPPED(status)) {
                    int job_id = add_job(pid, raw_command, JOB_STOPPED);
                    printf("\n[%d] Stopped\t%s\n", job_id, raw_command);
                    last_exit_status = 128 + WSTOPSIG(status);
                    break;
                }
            }
            // back to shell
            tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
            signal(SIGTTOU, SIG_DFL);
            set_pid_foreground(0);
            return last_exit_status;
        }
    }
}

// Main shell (interactive mode)
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
    init_jobs();
    while (1) {
        printf("Wah  ࣪𖤐. ");
        fgets(buffer, 255, stdin);
        check_jobs(); // check which jobs finished or stopped and print status
        if (strcmp(buffer, "\n") == 0 || strlen(buffer) == 0) {
            continue;
        }
        // !! command
        if (strncmp("!!", buffer, 2) == 0) {
            if (strlen(previous_buffer) == 0) {
                printf("No previous command.\n");
                continue;
            }
            printf("%s", previous_buffer);
            strncpy(buffer, previous_buffer, MAX_CMD_BUFFER - 1);
            buffer[MAX_CMD_BUFFER - 1] = '\0';
            last_exit_status = 0;
        }
        // check if background 
        int background = 0;
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 2] == '&') {
            background = 1;
            buffer[len - 2] = '\0';
        }
        // Background jobs and job control commands
        if (strncmp("jobs", buffer, 4) == 0) {
            jobs_command();
            last_exit_status = 0;
            continue;
        } else if (strncmp("fg ", buffer, 3) == 0) {
            fg_command(buffer + 3);
            continue;
        } else if (strncmp("bg ", buffer, 3) == 0) {
            bg_command(buffer + 3);
            continue;
        } 
        // echo commands
        else if ((strncmp(buffer, "echo",4) == 0) && (is_echo_output(buffer)!=1)) { // check if it for redirect or not
            if (strncmp("echo $?", buffer, 7) == 0) { // print last exit status
                printf("%d\n", last_exit_status);
            } else {
                int length = strlen(buffer) - 5;
                char text[length + 1];
                strncpy(text, buffer + 5, length);
                text[length] = '\0';
                printf("%s", text);
                last_exit_status = 0;
            }
        }
        // exit command 
        else if (strncmp("exit ", buffer, 4) == 0) {
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
        } else { // other than built in commands
            char buffer_copy[MAX_CMD_BUFFER];
            strcpy(buffer_copy, buffer); // copy original buffer
            strcat(buffer_copy, "\n");
            external_program(buffer_copy, background);
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
    return 0;
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

