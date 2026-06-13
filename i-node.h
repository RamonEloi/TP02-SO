#ifndef I_NODE_H
#define I_NODE_H

#include "common.h"

typedef struct inode {
    int id;                 
    char nome[50];          
    int tamanho;            
    int is_diretorio;       // 1 para diretório, 0 para arquivo 

    int blocos[12];        

    time_t criado;
    time_t modificado;
    time_t acessado;

    //Para todos abaixo, -1 indica "não tem", ou e o diretorio raiz
    int id_pai;            
    int id_primeiroFilho;   
    int id_proximoIrmao;    
} inode;

void guardar_inode(const char* nome_disco, int id_inode, inode* in);
void ler_inode(const char* nome_disco, int id_inode, inode* in);

#endif