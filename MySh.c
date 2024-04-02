#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_SIZE 1024
char cmd[MAX_SIZE];

int main (int argc, char* argv[])
{
    while(1){	
	char cwd[MAX_SIZE];
        // Obtem o diretório de trabalho atual
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            // perror("Erro ao obter diretório de trabalho atual");
           return EXIT_FAILURE;
        }
        // Exibe o prompt com [MySh] seguido do diretório de trabalho atual
        printf("[MySh] %s$ ", cwd);
        fflush(stdout);
        
        fgets(cmd, MAX_SIZE, stdin);
        cmd[strcspn(cmd, "\n")] = '\0';

	char *args[MAX_SIZE];
        char *tokens = strtok(cmd," ");

	int i = 0;
	while(tokens != NULL && i< MAX_SIZE){
	    args[i++] = tokens;
            tokens = strtok(NULL, " ");	    
	}
	args[i] = NULL;

        if(strcmp(cmd,"exit")==0){
	    return EXIT_SUCCESS;
	}else {
            pid_t pid = fork();
            
            if (pid == 0) {
                execvp(args[0], args);
                perror("Error: No such file or directory.");
                exit(EXIT_FAILURE);
            } else if (pid > 0) {
                wait(NULL); // Espera pelo término do filho
            } else {
                // Erro no fork
                perror("Error: Cannot create process");
            }
        }	
    }
}

