#include "diretorio.h"
#include "i-node.h"
#include "disco.h"

void inicializar_diretorio_raiz(const char *nome_disco)
{
    // 1. Configurar o i-node 0 (A Raiz)
    inode raiz;
    raiz.id = 0;
    strcpy(raiz.nome, "/");
    raiz.is_diretorio = 1; 
    raiz.tamanho = 0;
    raiz.id_pai = 0;            
    raiz.id_primeiroFilho = -1; 
    raiz.id_proximoIrmao = -1;

    // A raiz vai usar o Bloco 0 de dados para guardar a sua lista de ficheiros
    raiz.blocos[0] = 0;

    for (int i = 1; i < 12; i++)
    {
        raiz.blocos[i] = -1;
    }

    FILE *f = fopen(nome_disco, "r+b");
    if (f == NULL)
        return;

    Disco sb;
    fread(&sb, sizeof(Disco), 1, f);
    
    sb.blocos_livres--;
    sb.inodes_livres--;
    
    // Grava o Superbloco atualizado de volta no início
    fseek(f, 0, SEEK_SET);
    fwrite(&sb, sizeof(Disco), 1, f);

    // Carrega o bitmap, marca a posição 0 como ocupada (1) e grava de volta
    int *bitmap = (int*) malloc(sb.qtd_blocos * sizeof(int));
    fseek(f, sizeof(Disco), SEEK_SET);
    fread(bitmap, sizeof(int), sb.qtd_blocos, f);
    
    bitmap[0] = 1; // Bloco 0 agora está oficialmente ocupado
    
    fseek(f, sizeof(Disco), SEEK_SET);
    fwrite(bitmap, sizeof(int), sb.qtd_blocos, f);
    free(bitmap);

    // Quantidade de entradas (Tamanho do bloco / Tamanho de uma entrada)
    int qtd_entradas = sb.tamanho_bloco / sizeof(EntradaDiretorio);
    EntradaDiretorio *entradas_vazias = (EntradaDiretorio *)malloc(qtd_entradas * sizeof(EntradaDiretorio));

    for (int i = 0; i < qtd_entradas; i++)
    {
        entradas_vazias[i].id_inode = -1;
        strcpy(entradas_vazias[i].nome, "");
    }

    // Calcula onde começam os blocos de dados
    // Salta Superbloco + Mapa de Bits + Todos os Inodes
    long inicio_blocos_dados = sizeof(Disco) + (sb.qtd_blocos * sizeof(int)) + (sb.qtd_inodes * sizeof(inode));

    // O bloco 0 fica exatamente no início da área de dados
    fseek(f, inicio_blocos_dados, SEEK_SET);
    fwrite(entradas_vazias, sizeof(EntradaDiretorio), qtd_entradas, f);

    free(entradas_vazias);
    fclose(f);

    guardar_inode(nome_disco, 0, &raiz);
    printf("Diretorio Raiz '/' inicializado no i-node 0 com sucesso!\n");
}