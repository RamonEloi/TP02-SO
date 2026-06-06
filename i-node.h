#include "common.h"

typedef struct inode{

    int id;
    char nome[50];

    int tamanho;
    int diretorio;

    int blocos[12];

    time_t criado;
    time_t modificado;
    time_t acessado;

    struct inode *pai;
    struct inode *primeiroFilho;
    struct inode *proximoIrmao;


}inode;



