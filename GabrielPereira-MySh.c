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

        if(strcmp(cmd,"exit")==0){
            printf("Saindo...\n");
            return EXIT_SUCCESS;
        }

        // Divide os comandos pelo pipe '|'
        char *pipe_cmds[MAX_SIZE];
        int num_pipes = 0;
        pipe_cmds[num_pipes] = strtok(cmd, "|");
        while(pipe_cmds[num_pipes] != NULL) {
            pipe_cmds[++num_pipes] = strtok(NULL, "|");
        }

        int pipe_fds[2*num_pipes];
        for(int i = 0; i < num_pipes - 1; i++) {
            if (pipe(pipe_fds + i*2) < 0) {
                perror("Error creating pipe");
                exit(EXIT_FAILURE);
            }
        }
      
        int status;
        pid_t pid;
        int j = 0;
        for (int i = 0; i < num_pipes; i++) {
                // Divide o comando atual em argumentos
                char *args[MAX_SIZE];
                char *tokens = strtok(pipe_cmds[i]," ");
                int k = 0;
                while(tokens != NULL && k< MAX_SIZE){
                    args[k++] = tokens;
                    tokens = strtok(NULL, " ");      
                }
                args[k] = NULL;
      
        if(strcmp(args[0],"cd") == 0){
          if(args[1] == NULL || strcmp(args[1],"~") == 0){
            chdir(home);
          }else{
            if(chdir(args[1])!=0){
              perror("cd");
            }
          }
        }else{
            pid = fork();
            if (pid == 0) {
                // Redireciona a entrada do pipe anterior se não for o primeiro comando
                if (i != 0) {
                    if (dup2(pipe_fds[j - 2], 0) < 0) {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                }
                // Redireciona a saída para o próximo pipe se não for o último comando
                if (i != num_pipes - 1) {
                    if (dup2(pipe_fds[j + 1], 1) < 0) {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                }
                // Fecha todos os pipes no processo filho
                for (int k = 0; k < 2*(num_pipes - 1); k++) {
                    close(pipe_fds[k]);
                }

                // Executa o comando
                execvp(args[0], args);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else if (pid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            j += 2;
        }
        }

        // Fecha todos os pipes no processo pai
        for (int i = 0; i < 2 * (num_pipes - 1); i++) {
            close(pipe_fds[i]);
        }

        // Espera por todos os processos filhos
        for (int i = 0; i < num_pipes; i++) {
            wait(&status);
        }
    }
}
