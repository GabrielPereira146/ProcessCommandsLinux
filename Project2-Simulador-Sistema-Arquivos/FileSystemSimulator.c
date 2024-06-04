#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Códigos de escape ANSI para cores e estilos
#define RESET "\033[0m"
#define BOLD "\033[1m"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[93m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
//----------------------------------------------

void showMenu(){
	printf(BOLD CYAN "MENU\n");
	printf(BOLD CYAN "----------------\n");
	printf(BOLD YELLOW "criad     ->   ") ;
	printf(BOLD "Create a directory\n" );

	printf(BOLD YELLOW "criaa     ->   ") ;
	printf(BOLD "Create a file\n" );


	printf(BOLD MAGENTA "removed   ->   ") ;
	printf(BOLD "Remove a directory\n");

	printf(BOLD MAGENTA "removea   ->   ") ;
	printf(BOLD "Remove a file\n");


	printf(BOLD GREEN "verd      ->   ") ;
	printf(BOLD "Show files and subdirectories\n");

	printf(BOLD GREEN "verset    ->   ") ;
	printf(BOLD "Show sectors of file\n");

	printf(BOLD GREEN "map       ->   ") ;
	printf(BOLD "Show occupation of sectors\n" );

	printf(BOLD GREEN "tree      ->   ") ;
	printf(BOLD "Show file system tree\n");

	printf(BOLD CYAN "help      ->   ") ;
	printf(BOLD "Show options menu\n");

	printf(BOLD RED "exit      ->   ") ;
	printf(BOLD "Close program\n\n" RESET);


}


#define SECTOR_SIZE 512
#define NUM_SECTORS 256
#define NUM_RESERVED_SECTORS 10
#define MAX_SIZE 1024

// Estrutura para o diretório ou arquivo
typedef struct Entry {
	char name[100];
	int start_sector;
	int size;
	int is_directory;
	int quant_childs;
	struct Entry *subdirectories[NUM_SECTORS];
} Entry;

// 
int bitmap[NUM_SECTORS];

// Tabela de alocação de blocos
int fat[NUM_SECTORS];

// Diretório raiz
Entry root_directory;

void init_disk() {
	// Inicializar o mapa de bits indicando que todos os setores estão livres
	for (int i = 0; i < NUM_SECTORS; i++) {
		if(i<10) // setor de boot
			bitmap[i] = 1; // 1 indica setor ocupado
		else
			bitmap[i] = 0; // 0 indica setor livre
	}

	// Inicializar a tabela de alocação de blocos
	for (int i = NUM_RESERVED_SECTORS; i < NUM_SECTORS; i++) {
		fat[i] = -1; // -1 indica setor livre
	}

	// Inicializar o diretório raiz
	strcpy(root_directory.name, "root");
	root_directory.start_sector = 6;
	root_directory.size = 1;
	root_directory.is_directory = 1;
	root_directory.quant_childs = 0;
	//root_directory.subdirectories = NULL;
}

void create_directory(char *path) {
	Entry *parent = &root_directory;

	char *paths_dir[MAX_SIZE];
	int num_dir = 0;

	paths_dir[num_dir] = strtok(path, "\\");
	while(paths_dir[num_dir] != NULL) {
		paths_dir[++num_dir] = strtok(NULL, "\\");
	}
	
	for (int i = 2; i < num_dir; i++) {
		Entry *child = NULL;
		for (int j = 0; j < parent->quant_childs; j++) {
			if (strcmp(paths_dir[i-1], parent->subdirectories[j]->name) == 0) {
				child = parent->subdirectories[j];
				break;
			}
		}
		if (child == NULL ) {
			printf(BOLD RED "Erro: O diretório pai especificado não existe.\n" RESET);
			return; // Retorna sem criar o diretório
		}
		parent = child; // Atualiza o pai para o próximo diretório
	}

	// Verifica se o diretório a ser criado já existe dentro do diretório pai
	for (int j = 0; j < parent->quant_childs; j++) {
		if (strcmp(paths_dir[num_dir - 1], parent->subdirectories[j]->name) == 0) {
			printf(BOLD RED "Erro: O diretório a ser criado já existe no diretório pai.\n" RESET);
			return; // Retorna sem criar o diretório
		}
	}

	// Se chegou até aqui, o diretório pai existe e o diretório a ser criado não existe dentro dele
	// Podemos criar o diretório filho
	Entry *new_directory = (Entry *)malloc(sizeof(Entry));
	strcpy(new_directory->name, paths_dir[num_dir-1]); // Último elemento do caminho
	new_directory->start_sector = 0; // Defina o setor inicial adequadamente
	new_directory->size = 1; // Defina o tamanho inicial adequadamente
	new_directory->is_directory = 1;
	new_directory->quant_childs = 0;
	for (int k = 0; k < NUM_SECTORS; k++) {
		new_directory->subdirectories[k] = NULL;
	}

	// Adiciona o novo diretório à lista de subdiretórios do pai
	parent->subdirectories[parent->quant_childs++] = new_directory;

	for(int i = 10; i < NUM_SECTORS; i++){
		if(bitmap[i] == 0){
			bitmap[i] = 1;
			new_directory->start_sector = i;
			break;
		}
	}
	if(new_directory->start_sector == 0){
		printf(BOLD RED "Erro: Sem espaço em disco para criar diretorio.\n" RESET);
	}
	printf(BOLD GREEN "Diretório criado com sucesso.\n" RESET);
}

int remove_directory(char *path) {
	Entry *parent = &root_directory;
	char *path_sub = malloc(MAX_SIZE * sizeof(char));;
	strcpy(path_sub,path);
	strcat(path_sub,"\\");
	char *paths_dir[MAX_SIZE];
	int num_dir = 0;

	paths_dir[num_dir] = strtok(path, "\\");
	while(paths_dir[num_dir] != NULL) {
		paths_dir[++num_dir] = strtok(NULL, "\\");
	}
	Entry *child = NULL;
	fflush(stdout);
	for (int i = 2; i < num_dir; i++) {
		child = NULL;
		for (int j = 0; j < parent->quant_childs; j++) {
			if (strcmp(paths_dir[i-1], parent->subdirectories[j]->name) == 0) {
				child = parent->subdirectories[j];
				break;
			}
		}
		if (child == NULL ) {
			printf(BOLD RED "Erro: O diretório pai especificado não existe.\n" RESET);
			return 1; // Retorna sem deletar o diretório
		}
		parent = child; // Atualiza o pai para o próximo diretório
	}

	// Verifica se o diretório a ser deletado existe dentro do diretório pai
	char *aux = malloc(MAX_SIZE * sizeof(char)); //variavel para concatenar com o pathsub sem altera-lo
	for (int j = 0; j < parent->quant_childs; j++) {

		if (strcmp(paths_dir[num_dir - 1], parent->subdirectories[j]->name) == 0) {
			child = parent->subdirectories[j];

			for(int i = 0; i< child->quant_childs; i++){
				fprintf(stderr,"%s",path_sub);
				strcpy(aux, path_sub);

				strcat(aux,child->subdirectories[i]->name);

				//remove_directory(aux);
			}
		
			parent->subdirectories[j] = parent->subdirectories[parent->quant_childs];
			parent->quant_childs--;
			free(aux);
			free(path_sub);
			return 0;
		}
	}
        free(aux);
	free(path_sub);
	printf(BOLD RED "Erro: O diretório a ser deletado não existe no diretório pai.\n" RESET);
	return 1; // Retorna sem deletar o diretório


}

int main(int argc, char **argv) {
	init_disk();

	char input[MAX_SIZE];

	while(1){
		// Recebe a entrada do teclado
		//printf(BOLD CYAN "----------------\n" RESET);
		printf(BOLD CYAN "↪ " RESET);
		fgets(input, MAX_SIZE, stdin);

		// Remove o \n do final da entrada
		input[strcspn(input, "\n")] = '\0';

		char *args[MAX_SIZE];
		int i = 0;
		args[i] = strtok(input, " ");
		while(args[i] != NULL)
			args[++i] = strtok(NULL, " "); 


		if (strcmp(args[0], "criad") == 0){
			if(args[1] != NULL)
				create_directory(args[1]);
			else
				printf(BOLD "Erro Sintaxe\n" RESET);
		}else if (strcmp(args[0], "criaa") == 0){
			printf(BOLD "Making file...\n" RESET);
		}else if (strcmp(args[0], "removed") == 0){

			if(args[1] != NULL){
				if(remove_directory(args[1]) == 0){
					printf(BOLD GREEN "Diretório deletado com sucesso.\n" RESET);
				}
			}
		}else if (strcmp(args[0], "removea") == 0){
			printf(BOLD "Removing file...\n" RESET);
		}else if (strcmp(args[0], "exit") == 0){
			printf(BOLD "Closing the simulator...\n" RESET);
			return EXIT_SUCCESS;			
		}else if(strcmp(args[0], "help") == 0){
			showMenu();
		}
		else{
			printf(BOLD"command not found...\n");
			printf(BOLD "For more information about the commands, type 'help'\n");
		}
	}
	return 0;
}

