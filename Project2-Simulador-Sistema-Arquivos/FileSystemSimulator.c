#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE 512
#define NUM_SECTORS 256
#define NUM_RESERVED_SECTORS 10

// Estrutura para o diretório ou arquivo
typedef struct {
    char name[100];
    int start_sector;
    int size;
    int is_directory;
} FileEntry;

// Diretorio do usuario
int bitmap[NUM_SECTORS];

// Tabela de alocação de blocos
int fat[NUM_SECTORS];

// Diretório raiz
FileEntry root_directory;

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
    strcpy(root_directory.name, "/");
    root_directory.start_sector = 6;
    root_directory.size = 1;
    root_directory.is_directory = 1;
}

int main() {
    init_disk();
    // Aqui você pode adicionar as funções para criar arquivos, diretórios, etc.
    return 0;
}

