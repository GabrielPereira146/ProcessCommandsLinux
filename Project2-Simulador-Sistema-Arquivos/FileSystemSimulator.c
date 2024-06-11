#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Códigos de escape ANSI para cores e estilos
#define RESET "\033[0m"
#define BOLD "\033[1m"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[93m"
#define BLUE "\033[34m"
#define BLUE_LIGHT "\033[94m"
#define BLUE_BRIGHT "\033[1;34m"
#define BLUE_DARK "\033[34;1m"
#define BLUE_VIVID "\033[94m"
#define CYAN "\033[36m"
#define MAGENTA "\033[35m"
#define WHITE "\033[37m"
//----------------------------------------------

int remove_file(char *path);

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
  char *root = malloc(MAX_SIZE * sizeof(char));
  strcpy(root,"root");
	char *paths_dir[MAX_SIZE];
	int num_dir = 0;
  
if(strncmp(path,"\\",1)==0){
    strcat(root,path);
    strcpy(path,root);
  }else if(strncmp(path,"root\\",5)!=0){
    strcat(root,"\\");
    strcat(root,path);
    strcpy(path,root);
}
	paths_dir[num_dir] = strtok(path, "\\");
	while(paths_dir[num_dir] != NULL) {
		paths_dir[++num_dir] = strtok(NULL, "\\");
	}

	
  for (int i = 2; i < num_dir; i++) {
    Entry *child = malloc(sizeof(Entry));
    child = NULL;
		for (int j = 0; j < parent->quant_childs; j++) {
			if (strcmp(paths_dir[i-1], parent->subdirectories[j]->name) == 0) {
        if((parent->is_directory)==1)
				  child = parent->subdirectories[j];
				break;
			}
		}
    
		if (child == NULL ) {
			printf(BOLD RED "Erro: O diretório pai especificado não existe.\n" RESET);
      free(root);
      free(child);
			return; // Retorna sem criar o diretório
		}
    parent = child; // Atualiza o pai para o próximo diretório
  }

	// Verifica se o diretório a ser criado já existe dentro do diretório pai
	for (int j = 0; j < parent->quant_childs; j++) {
		if (strcmp(paths_dir[num_dir - 1], parent->subdirectories[j]->name) == 0) {
			printf(BOLD RED "Erro: O diretório a ser criado já existe no diretório pai.\n" RESET);
			free(root);
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
    return;	
  }
  free(root);
	printf(BOLD GREEN "Diretório criado com sucesso.\n" RESET);
}

int remove_directory(char *path) {
	Entry *parent = &root_directory;
	char *path_sub = malloc(MAX_SIZE * sizeof(char));
  
  char *root = malloc(MAX_SIZE * sizeof(char));
  strcpy(root,"root");

  if(strncmp(path,"\\",1)==0){
    strcat(root,path);
    strcpy(path,root);
  }else if(strncmp(path,"root\\",5)!=0){
    strcat(root,"\\");
    strcat(root,path);
    strcpy(path,root);
  }
  
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
        if((parent->is_directory)==1)
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
				fprintf(stderr,"%s\n",path_sub);
				strcpy(aux, path_sub);

				strcat(aux,child->subdirectories[i]->name);
        if(child->subdirectories[i]->is_directory){
          if(remove_directory(aux)){
            return 1;
          }
        } else {
          if(remove_file(aux)){
            return 1;
          }
        }
				
			}
		  bitmap[child->start_sector]=0;
			parent->subdirectories[j] = parent->subdirectories[parent->quant_childs-1];
			parent->quant_childs--;
      free(root);
			free(aux);
			free(path_sub);
			return 0;
		}
	}
  free(root);
  free(aux);
	free(path_sub);
	printf(BOLD RED "Erro: O diretório a ser deletado não existe no diretório pai.\n" RESET);
	return 1; // Retorna sem deletar o diretório


}

void create_file(char *path, float size) {
	Entry *parent = &root_directory;
  char *root = malloc(MAX_SIZE * sizeof(char));
  strcpy(root,"root");
	char *paths_dir[MAX_SIZE];
	int num_dir = 0;
  
if(strncmp(path,"\\",1)==0){
    strcat(root,path);
    strcpy(path,root);
  }else if(strncmp(path,"root\\",5)!=0){
    strcat(root,"\\");
    strcat(root,path);
    strcpy(path,root);
}
	paths_dir[num_dir] = strtok(path, "\\");
	while(paths_dir[num_dir] != NULL) {
		paths_dir[++num_dir] = strtok(NULL, "\\");
	}

	
  for (int i = 2; i < num_dir; i++) {
    Entry *child = malloc(sizeof(Entry));
    child = NULL;
		for (int j = 0; j < parent->quant_childs; j++) {
			if (strcmp(paths_dir[i-1], parent->subdirectories[j]->name) == 0) {
        if((parent->is_directory)==1)
				  child = parent->subdirectories[j];
				break;
			}
		}
    
		if (child == NULL ) {
			printf(BOLD RED "Erro: O diretório pai especificado não existe.\n" RESET);
      free(root);
      free(child);
			return; // Retorna sem criar o arquivo
		}
    parent = child; // Atualiza o pai para o próximo diretório
  }

	// Verifica se o diretório a ser criado já existe dentro do diretório pai
	for (int j = 0; j < parent->quant_childs; j++) {
		if (strcmp(paths_dir[num_dir - 1], parent->subdirectories[j]->name) == 0) {
			printf(BOLD RED "Erro: O arquivo a ser criado já existe no diretório pai.\n" RESET);
			free(root);
      return; // Retorna sem criar o diretório
		} 
	}

	// Se chegou até aqui, o diretório pai existe e o arquivo a ser criado não existe dentro dele
	// Podemos criar o arquivo filho
	Entry *new_file = (Entry *)malloc(sizeof(Entry));
	strcpy(new_file->name, paths_dir[num_dir-1]); // Último elemento do caminho
	new_file->start_sector = 0; // Defina o setor inicial adequadamente
	new_file->size = size; // Defina o tamanho inicial adequadamente
	new_file->is_directory = 0;
	new_file->quant_childs = 0;
	for (int k = 0; k < NUM_SECTORS; k++) {
		new_file->subdirectories[k] = NULL;
	}

	// Adiciona o novo arquivo à lista de subdiretórios do pai
	parent->subdirectories[parent->quant_childs++] = new_file;
  double num_blocks = size/SECTOR_SIZE;
  num_blocks = ceil(num_blocks);
  int old_i = -1;
  for(int j = 0; j<num_blocks; j++){
    for(int i = 10; i < NUM_SECTORS; i++){
      if(bitmap[i] == 0){
        bitmap[i] = 1;
        if(j==num_blocks-1){
          fat[i] == -1;
        }
        if(j==0){
          new_file->start_sector = i;
        }else{
          fat[old_i] = i;
        }
        old_i = i;
        break;
      }
    }
  }
	if(new_file->start_sector == 0){
		printf(BOLD RED "Erro: Sem espaço em disco para criar arquivo.\n" RESET);
    return;	
  }
  free(root);
	printf(BOLD GREEN "Arquivo criado com sucesso.\n" RESET);
}

int remove_file(char *path){
	Entry *parent = &root_directory;
	char *path_sub = malloc(MAX_SIZE * sizeof(char));
  
  char *root = malloc(MAX_SIZE * sizeof(char));
  strcpy(root,"root");

  if(strncmp(path,"\\",1)==0){
    strcat(root,path);
    strcpy(path,root);
  }else if(strncmp(path,"root\\",5)!=0){
    strcat(root,"\\");
    strcat(root,path);
    strcpy(path,root);
  }
  
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
        if((parent->is_directory)==1)
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
	// Verifica se o arquivo a ser deletado existe dentro do diretório pai
	char *aux = malloc(MAX_SIZE * sizeof(char)); //variavel para concatenar com o pathsub sem altera-lo
	for (int j = 0; j < parent->quant_childs; j++) {

		if (strcmp(paths_dir[num_dir - 1], parent->subdirectories[j]->name) == 0) {
			child = parent->subdirectories[j];
      int num_blocks = ceil((float)child->size / (float)SECTOR_SIZE);
		  bitmap[child->start_sector]=0;
       
      int new_i = child->start_sector;
      while (new_i != -1){
       int temp = fat[new_i];
       fat[new_i] = -1;
       bitmap[new_i] = 0;
       new_i = temp;
      }
			parent->subdirectories[j] = parent->subdirectories[parent->quant_childs-1];
			parent->quant_childs--;
      free(root);
			free(aux);
			free(path_sub);
			return 0;
		}
	}
  free(root);
  free(aux);
	free(path_sub);
	printf(BOLD RED "Erro: O diretório a ser deletado não existe no diretório pai.\n" RESET);
	return 1; // Retorna sem deletar o diretório
}

// int walkthroughTree(Entry *parent, int j){
//   for (int i = 0; i < parent->quant_childs; i++) {
//         walkthroughTree(parent->subdirectories[i],j);
//   }
//   j+=parent->quant_childs;
//   return j;
// }

int walkthroughTree(Entry *parent, int j){
  
  for (int i = 0; i < parent->quant_childs; i++) {
        j=+walkthroughTree(parent->subdirectories[i],j);
  }
  j+=parent->quant_childs;
  return j;
}

void view_sector(char *path){
  Entry *parent = &root_directory;
  char *root = malloc(MAX_SIZE * sizeof(char));
  strcpy(root,"root");
	char *paths_dir[MAX_SIZE];
	int num_dir = 0;
  
if(strncmp(path,"\\",1)==0){
    strcat(root,path);
    strcpy(path,root);
  }else if(strncmp(path,"root\\",5)!=0){
    strcat(root,"\\");
    strcat(root,path);
    strcpy(path,root);
}
	paths_dir[num_dir] = strtok(path, "\\");
	while(paths_dir[num_dir] != NULL) {
		paths_dir[++num_dir] = strtok(NULL, "\\");
	}

	
  for (int i = 2; i < num_dir; i++) {
    Entry *child = malloc(sizeof(Entry));
    child = NULL;
		for (int j = 0; j < parent->quant_childs; j++) {
			if (strcmp(paths_dir[i-1], parent->subdirectories[j]->name) == 0) {
        if((parent->is_directory)==1)
				  child = parent->subdirectories[j];
				break;
			}
		}
    
		if (child == NULL ) {
			printf(BOLD RED "Erro: O diretório pai especificado não existe.\n" RESET);
      free(root);
      free(child);
			return; // Retorna sem criar o arquivo
		}
    parent = child; // Atualiza o pai para o próximo diretório
  }

	// Procura o arquivo dentro do diretório pai
	for (int j = 0; j < parent->quant_childs; j++) {
		if (strcmp(paths_dir[num_dir - 1], parent->subdirectories[j]->name) == 0) {
      int new_i = parent->subdirectories[j]->start_sector;
      while (new_i != -1){
        printf(BOLD " %d" RESET, new_i);
        printf(BOLD CYAN " ➜ " RESET);
        new_i = fat[new_i];
      }
      printf("NULL \n");
      
			free(root);
		} 
	}
}

int showDataInfo(char *path){
  Entry *parent = &root_directory;
  char *path_sub = malloc(MAX_SIZE * sizeof(char));
  int count = 0;
  char *root = malloc(MAX_SIZE * sizeof(char));
  strcpy(root,"root");

  if(strncmp(path,"\\",1)==0){
    strcat(root,path);
    strcpy(path,root);
  }else if(strncmp(path,"root\\",5)!=0){
    strcat(root,"\\");
    strcat(root,path);
    strcpy(path,root);
  }

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
        if((parent->is_directory)==1)
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
  strcpy(root,"root");
  
  if(strcmp(paths_dir[num_dir - 1],root) != 0){
    for (int j = 0; j < parent->quant_childs; j++) {
      if (strcmp(paths_dir[num_dir - 1], parent->subdirectories[j]->name) == 0) {
        child = parent->subdirectories[j];
        break;
      }
    }
    parent = child;
  }
  
  
  if (!parent->is_directory){
     printf(BOLD RED "Erro: O diretório a ser exibido não existe.\n" RESET);
     return 1;
  }
  // Verifica se o diretório a ser deletado existe dentro do diretório pai
    printf("Nome                            | Tipo       | Tamanho | Criado em           \n");
    printf("-----------------------------------------------------------------------------\n");

//   typedef struct Entry {
// 	char name[100];
// 	int start_sector;
// 	int size;
// 	int is_directory;
// 	int quant_childs;
// 	struct Entry *subdirectories[NUM_SECTORS];
// } Entry;

    char typeFile[12];

    for (int j = 0; j < parent->quant_childs; j++) {
        Entry *child = parent->subdirectories[j];
        
        // Imprimir o nome
        printf("%-32s", child->name);
        
        // Imprimir o tipo
        if (child->is_directory) {
            strcpy(typeFile, "Diretório");
            printf(BOLD CYAN "| %-11s " RESET, typeFile);
        } else {
            strcpy(typeFile, "Arquivo");
            printf("| %-10s ", typeFile);
        }
        

        // Imprimir o tamanho
        printf("| %-7d ", child->size);

        // Imprimir a data de criação
        printf("| Data\n");
    }
    
    
//     for(int i = 0; i< child->quant_childs; i++){
//       fprintf(stderr,"%s\n",path_sub);
//       // 				strcpy(aux, path_sub);

//       // 				strcat(aux,child->subdirectories[i]->name);
//       // 				if(remove_directory(aux)){
//       //           printf("ERRO ERRO ERRO\n");
//       //           return 1;
//       //         }
//     }
//     bitmap[child->start_sector]=0;
//     printf("<removed> Posicao no Bitmap: %d Bitmap: %d\n", child->start_sector, bitmap[child->start_sector]);
//     parent->subdirectories[j] = parent->subdirectories[parent->quant_childs-1];
//     parent->quant_childs--;
//     free(root);
//     // 			free(aux);
//     free(path_sub);
//     return 0;
  
  count = walkthroughTree(&root_directory, 1);
  printf("\nTotal de arquivos e diretórios %d\n", count);
  count = 0;
  for (int i = 0; i < NUM_SECTORS; i++) {
    if (bitmap[i] == 1){
      count++;
    }
  }
  int TotalSize;
  TotalSize = NUM_SECTORS * SECTOR_SIZE;
  count = count * SECTOR_SIZE;
  printf("Espaço ocupado %d bytes\n", count);
  printf("Espaço livre %d bytes\n", (TotalSize-count));
  free(root);
  //   free(aux);
  free(path_sub);
  //printf(BOLD RED "Erro: O diretório a ser deletado não existe no diretório pai.\n" RESET);
  return 1; // Retorna sem deletar o diretório

}

void showMap(){
  for (int i = 0; i < NUM_SECTORS; i++) {
    printf("|   Setor %3d   ", i);
    if ((i + 1) % 16 == 0) {
      printf("|\n");
      for (int j = i - 15; j <= i; j++) {
        if (bitmap[j] == 1) {
          printf("|    Ocupado    ");
        } else {
          printf("|     Livre     ");
        }
      }
      printf("|\n\n");
    }
  }


}

void showTree(Entry *parent, int depth, int isLast[]){
  for (int i = 0; i < depth; i++) {
        if (i < depth - 1) {
            if (isLast[i]) {
                printf("    ");
            } else {
                printf("│   ");
            }
        } else {
            if (isLast[i]) {
                printf("└── ");
            } else {
                printf("├── ");
            }
        }
  }
  if(parent->is_directory)
    printf(BOLD CYAN "%s\n" RESET, parent->name);
  else
    printf("%s\n", parent->name);
  for (int i = 0; i < parent->quant_childs; i++) {
        isLast[depth] = (i == parent->quant_childs - 1);
        showTree(parent->subdirectories[i], depth + 1, isLast);
  }
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
      if((args[1] != NULL)&&(args[2] != NULL)){
        create_file(args[1], atof(args[2]));
      }
		}else if (strcmp(args[0], "removed") == 0){

			if(args[1] != NULL){
				if(remove_directory(args[1]) == 0){
					printf(BOLD GREEN "Diretório deletado com sucesso.\n" RESET);
				}
			}
		}else if (strcmp(args[0], "removea") == 0){
			if(args[1] != NULL){
        if(!remove_file(args[1])){
          printf(BOLD GREEN "Arquivo deletado com sucesso.\n" RESET);
        }
      }
		}else if (strcmp(args[0], "verd") == 0){
			if(args[1] != NULL){
        showDataInfo(args[1]);
      }
		}else if (strcmp(args[0], "verset") == 0){
			if(args[1] != NULL){
        view_sector(args[1]);
      }
		}else if(strcmp(args[0], "arvore") == 0){
      int isLast[MAX_SIZE] = {0};
      showTree(&root_directory,0,isLast);
    }
    else if(strcmp(args[0], "mapa") == 0){
      showMap();
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

