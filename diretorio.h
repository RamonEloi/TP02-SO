#ifndef DIRETORIO_H
#define DIRETORIO_H

#include "common.h"

typedef struct {
    char nome[50];  
    int id_inode;   // (-1 indica que este espaço está vazio)
} EntradaDiretorio;

void inicializar_diretorio_raiz(const char *nome_disco);
#endif