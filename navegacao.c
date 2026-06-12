#include <stdio.h>
#include <string.h>
#include <time.h>

#include "navegacao.h"

int encontrar_inode_por_caminho(const char* nome_disco, const char* caminho, int id_diretorio_atual) {
    if (strcmp(caminho, "/") == 0) return 0; // Assume que o i-node 0 é a raiz

    char caminho_copia[256];
    strncpy(caminho_copia, caminho, sizeof(caminho_copia));
    
    int id_atual = id_diretorio_atual;
    if (caminho[0] == '/') {
        id_atual = 0; // Caminho absoluto começa na raiz
    }

    char* token = strtok(caminho_copia, "/");
    inode in_atual;

    while (token != NULL) {
        ler_inode(nome_disco, id_atual, &in_atual);
        
        if (!in_atual.is_diretorio) return -1; // O caminho tenta entrar num ficheiro que não é diretoria

        int id_filho = in_atual.id_primeiroFilho;
        int encontrou = 0;
        inode in_filho;

        // Itera sobre todos os filhos (irmãos do primeiro filho)
        while (id_filho != -1) {
            ler_inode(nome_disco, id_filho, &in_filho);
            if (strcmp(in_filho.nome, token) == 0) {
                id_atual = id_filho; 
                encontrou = 1;
                break;
            }
            id_filho = in_filho.id_proximoIrmao; 
        }

        if (!encontrou) return -1;
        token = strtok(NULL, "/");
    }

    return id_atual;
}

// Cria uma nova diretoria dentro de um diretório pai
int simular_mkdir(const char* nome_disco, const char* nome_novo_dir, int id_pai) {
    int novo_id = alocar_inode(nome_disco);
    if (novo_id == -1) {
        printf("Erro: Sem i-nodes livres.\n");
        return -1;
    }

    inode novo_dir;
    novo_dir.id = novo_id;
    strncpy(novo_dir.nome, nome_novo_dir, sizeof(novo_dir.nome) - 1);
    novo_dir.nome[sizeof(novo_dir.nome) - 1] = '\0';
    novo_dir.tamanho = 0;
    novo_dir.is_diretorio = 1;
    for(int i = 0; i < 12; i++) novo_dir.blocos[i] = -1;
    
    time_t agora = time(NULL);
    novo_dir.criado = agora;
    novo_dir.modificado = agora;
    novo_dir.acessado = agora;

    novo_dir.id_pai = id_pai;
    novo_dir.id_primeiroFilho = -1;
    novo_dir.id_proximoIrmao = -1;

    inode pai;
    ler_inode(nome_disco, id_pai, &pai);

    if (pai.id_primeiroFilho == -1) {
        pai.id_primeiroFilho = novo_id;
    } else {
        int id_irmao = pai.id_primeiroFilho;
        inode irmao;
        while (1) {
            ler_inode(nome_disco, id_irmao, &irmao);
            if (irmao.id_proximoIrmao == -1) {
                irmao.id_proximoIrmao = novo_id;
                guardar_inode(nome_disco, irmao.id, &irmao); // Atualiza o irmão no disco
                break;
            }
            id_irmao = irmao.id_proximoIrmao;
        }
    }

    pai.modificado = agora;
    
    guardar_inode(nome_disco, id_pai, &pai);
    guardar_inode(nome_disco, novo_id, &novo_dir);

    return novo_id;
}
void listar_diretorio(const char* nome_disco, int id_diretorio) {
    inode dir_atual;
    ler_inode(nome_disco, id_diretorio, &dir_atual);

    // Proteção de segurança
    if (!dir_atual.is_diretorio) {
        printf("Erro: O i-node %d nao e um diretorio.\n", id_diretorio);
        return;
    }

    int id_filho = dir_atual.id_primeiroFilho;

    if (id_filho == -1) {
        printf("Diretorio vazio.\n");
        return;
    }

    printf("TIPO\tID\tTAMANHO\tNOME\n");
    printf("----------------------------------------\n");


    while (id_filho != -1) {
        inode filho;
        ler_inode(nome_disco, id_filho, &filho);

        // Define se é Diretório (D) ou Arquivo (A)
        char tipo = filho.is_diretorio ? 'D' : 'A';
        
        printf("[%c]\t%d\t%d B\t%s\n", tipo, filho.id, filho.tamanho, filho.nome);
        id_filho = filho.id_proximoIrmao;
    }
}