#include "arquivo.h"

int criar_arquivo(const char *nome_disco, const char *nome_arquivo, int id_pai){
    int novo_id = alocar_inode(nome_disco);

    if(novo_id == -1)
    {
        printf("Erro: sem i-nodes livres.\n");
        return -1;
    }

    inode novo_arquivo;

    novo_arquivo.id = novo_id;

    strncpy(
        novo_arquivo.nome,
        nome_arquivo,
        sizeof(novo_arquivo.nome)-1
    );

    novo_arquivo.nome[sizeof(novo_arquivo.nome)-1] = '\0';

    novo_arquivo.tamanho = 0;

    novo_arquivo.is_diretorio = 0;

    for(int i=0;i<12;i++)
    {
        novo_arquivo.blocos[i] = -1;
    }

    time_t agora = time(NULL);

    novo_arquivo.criado = agora;
    novo_arquivo.modificado = agora;
    novo_arquivo.acessado = agora;

    novo_arquivo.id_pai = id_pai;

    novo_arquivo.id_primeiroFilho = -1;

    novo_arquivo.id_proximoIrmao = -1;

    inode pai;

    ler_inode(nome_disco, id_pai, &pai);

    if(pai.id_primeiroFilho == -1)
    {
        pai.id_primeiroFilho = novo_id;
    }
    else
    {
        int id_irmao = pai.id_primeiroFilho;

        inode irmao;

        while(1)
        {
            ler_inode(nome_disco,id_irmao,&irmao);

            if(irmao.id_proximoIrmao == -1)
            {
                irmao.id_proximoIrmao = novo_id;

                guardar_inode(
                    nome_disco,
                    irmao.id,
                    &irmao
                );

                break;
            }

            id_irmao = irmao.id_proximoIrmao;
        }
    }

    pai.modificado = agora;

    guardar_inode(
        nome_disco,
        id_pai,
        &pai
    );

    guardar_inode(
        nome_disco,
        novo_id,
        &novo_arquivo
    );

    return novo_id;
}