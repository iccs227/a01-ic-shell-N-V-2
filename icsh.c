/* ICCS227: Project 1: icsh
 * Name: Nuttapanee Visitatimat
 * StudentID: 6580975
 */

#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAX_CMD_BUFFER 255

void copy_buffer(char buffer[], char previous_buffer[], int n){
	if (n==0) return;
	previous_buffer[n-1] = buffer[n-1];
	copy_buffer(buffer,previous_buffer, n-1);
}
void toBinary(int num) {
	if (num>0){
		toBinary(num/2);
	}
}
int external_program(char buffer[]){
	int status;
	size_t len = strlen(buffer);
	while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == ' ')) {
    buffer[len-1] = '\0';
    len--;
	}
	char* p = strtok(buffer, " ");
	char* command[MAX_CMD_BUFFER /2 +1];
	int i = 0;
	while (p != NULL && i < MAX_CMD_BUFFER/2) { 
		command[i++] = p;
		p = strtok(NULL, " ");
	}
	command[i] = NULL;
	pid_t pid = fork();
	if (pid <  0) {
		perror("Fork failed");
		return 0;
	}
	if (pid == 0) {
		execvp(command[0], command);
		perror("execvp failed");
		exit(1);
	}else{
		waitpid(pid, &status,0);
		if (status == 0) { 
			return 1;
		} else {
			return 0;
		}
	}
}
int interactive_shell() { 
    char buffer[MAX_CMD_BUFFER];
    char previous_buffer[MAX_CMD_BUFFER];
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
    while (1) {
        printf("Wah  ࣪𖤐. ");
        fgets(buffer, 255, stdin);	
	if (strcmp(buffer, "\n") == 0){
		continue;
	}
	// 2. !!
	if (strncmp("!!", buffer, 2) == 0){
                printf("%s ", previous_buffer);
                copy_buffer(previous_buffer,buffer, sizeof(buffer));
	}
	// 1. echo
	if (strncmp("echo ", buffer, 5) == 0){
		int length = strlen(buffer) - 5;
		char text[length];
		strncpy(text, buffer+ 5, length);
		text[length] = '\0';
		printf("%s ", text);
	}
	//3. exit<num>
	else if (strncmp("exit ", buffer, 5) == 0 ) {
		int length = strlen(buffer) - 5;
                char text[length];
                strncpy(text, buffer+ 5, length);
                text[length] = '\0';
                int num = atoi(text);
		if (num > 255){
			num = num & 0xFF;
		}
		if(num == 1){
			printf("Bye ~~~~\n");
			printf(
       "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢎⠱⠊⡱⠀⠀⠀⠀⠀⠀\n"
       "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡠⠤⠒⠒⠒⠒⠤⢄⣑⠁⠀⠀⠀⠀⠀⠀⠀⠀\n"
       "⠀⠀⠀⠀⠀⠀⠀⢀⡤⠒⠝⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠲⢄⡀⠀⠀⠀⠀⠀\n"
       "⠀⠀⠀⠀⠀⢀⡴⠋⠀⠀⠀⠀⣀⠀⠀⠀⠀⠀⠀⢠⣢⠐⡄⠀⠉⠑⠒⠒⠒⣄\n"
       "⠀⠀⠀⣀⠴⠋⠀⠀⠀⡎⢀⣘⠿⠀⠀⢠⣀⢄⡦⠀⣛⣐⢸⠀⠀⠀⠀⠀⠀⢘\n"
       "⡠⠒⠉⠀⠀⠀⠀⠀⡰⢅⠣⠤⠘⠀⠀⠀⠀⠀⠀⢀⣀⣤⡋⠙⠢⢄⣀⣀⡠⠊\n"
       "⢇⠀⠀⠀⠀⠀⢀⠜⠁⠀⠉⡕⠒⠒⠒⠒⠒⠛⠉⠹⡄⣀⠘⡄⠀⠀⠀⠀⠀⠀\n"
       "⠀⠑⠂⠤⠔⠒⠁⠀⠀⡎⠱⡃⠀⠀⡄⠀⠄⠀⠀⠠⠟⠉⡷⠁⠀⠀⠀⠀⠀⠀\n"
       "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⠤⠤⠴⣄⡸⠤⣄⠴⠤⠴⠄⠼⠀⠀⠀⠀⠀⠀⠀⠀\n");

                        return 0;
		}else{
			printf("ERROR %d \n", num);
		}
	}
	else {
		if(!external_program(buffer)){ 
			printf("Bad Command!! \n");
		}
	}
	copy_buffer(buffer,previous_buffer, sizeof(buffer));
    }
}

int no_interactive_shell(char buffer[], char previous_buffer[]) {
	if (strcmp(buffer, "\n") == 0){
                return 0;
	}
	if (strncmp(buffer, "#", 1) == 0){
                return 0;
        }
        // 2. !!
        if (strncmp("!!", buffer, 2) == 0){
                no_interactive_shell(previous_buffer, buffer);
		return 0;
        }
        // 1. echo
        if (strncmp("echo ", buffer, 5) == 0){
                int length = strlen(buffer) - 5;
                char text[length];
                strncpy(text, buffer+ 5, length);
                text[length] = '\0';
                printf("%s", text);
		return 0;
        }
        //3. exit<num>
	if (strncmp("exit ", buffer, 5) == 0 ) {
                int length = strlen(buffer) - 5;
                char text[length];
                strncpy(text, buffer+ 5, length);
                text[length] = '\0';
                int num = atoi(text);
		if(num > 255){
                	num = num & 0xFF;
		}
                return num;
        }
	printf("Bad Command!!\n");
	return 0;
}



int main(int argc, char *argv[]){
	
	if(argc >= 2){
		FILE *file;
		char buffer[MAX_CMD_BUFFER];
		char previous_buffer[MAX_CMD_BUFFER];
		int ret;
		file = fopen(argv[1], "r");
		while(fgets(buffer, MAX_CMD_BUFFER, file)){
			ret = no_interactive_shell(buffer,previous_buffer);
			copy_buffer(buffer,previous_buffer, sizeof(buffer));;
		}
		fclose(file);
		return ret;
	}
	else{
		interactive_shell();
	}
	return 0;
}
