#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <signal.h>

#define MAX_SIZE 1024
char cmd[MAX_SIZE];

int main (int argc, char* argv[])
{
	signal(SIGTSTP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	char username[MAX_SIZE];
	char hostname[MAX_SIZE];
	char cwd[MAX_SIZE];
	char *home = getenv("HOME");
	char path[MAX_SIZE];

	while(1){
		// Obtem nome usario
		getlogin_r(username, MAX_SIZE);	
		/* Obtem o valor de hostname */
		gethostname(hostname,MAX_SIZE);
		// Obtem o diretório de trabalho atual
		if (getcwd(cwd, MAX_SIZE) == NULL) {
			perror("Erro ao obter diretório de trabalho atual");
			return EXIT_FAILURE;
		}
		// Altera home/user por ~
		strcpy(path, "~");
		char *pos_user = strstr(cwd, username);
		if(pos_user !=NULL){
			strcat(path, pos_user+strlen(username));
		}else{
			strcpy(path,cwd);
		}	
		// Exibe o prompt com [MySh] seguido do username, hostname e diretório de trabalho atual
		printf("[MySh] %s@%s:%s$ ", username,hostname, path);
		fflush(stdout);

		if(fgets(cmd, MAX_SIZE, stdin) == NULL){
			printf("\nSaindo...\n");
            		return EXIT_SUCCESS;
		}
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
			printf("Saindo...\n");
			return EXIT_SUCCESS;
		}else if(strcmp(cmd,"cd") == 0){
			if(args[1] == NULL || strcmp(args[1],"~") == 0){
				chdir(home);
			}else{
				if(chdir(args[1])!=0){
					perror("cd");
				}
			}
		}else{
			pid_t pid = fork();

			if (pid == 0) 
			{
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
