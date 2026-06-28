#include "arquivo.h"
#include "navegacao.h"

int criar_arquivo(const char *nome_disco, const char *nome_arquivo, int id_pai){
    if(buscar_filho_por_nome(nome_disco, id_pai, nome_arquivo) != -1) {
        printf("Erro: Arquivo '%s' ja existe no diretorio pai.\n", nome_arquivo);
        return -1;
    }

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

int importar_arquivo_real(const char *nome_disco, const char *caminho_real, const char *nome_virtual, int id_pai){
    FILE *arquivo_real = fopen(caminho_real, "rb");
    if (arquivo_real == NULL) {
        printf("Erro: Arquivo real '%s' nao encontrado.\n", caminho_real);
        return -1;
    }

    fseek(arquivo_real, 0, SEEK_END);
    long tamanho_real = ftell(arquivo_real);
    fseek(arquivo_real, 0, SEEK_SET);

    FILE *f_disco = fopen(nome_disco, "r+b");
    Disco sb;
    fread(&sb, sizeof(Disco), 1, f_disco);
    fclose(f_disco);

    long tamanho_maximo = 12 * sb.tamanho_bloco;
    if (tamanho_real > tamanho_maximo) {
        printf("Erro: O arquivo real é muito grande (%ld bytes). O limite atual é %ld bytes.\n", tamanho_real, tamanho_maximo);
        fclose(arquivo_real);
        return -1;
    }

    int id_inode = criar_arquivo(nome_disco, nome_virtual, id_pai);
    if (id_inode == -1) {
        fclose(arquivo_real);
        return -1;
    }
    inode meu_arquivo;
    ler_inode(nome_disco, id_inode, &meu_arquivo);
    meu_arquivo.tamanho = tamanho_real;

    char *buffer = (char *)malloc(sb.tamanho_bloco);
    int blocos_usados = 0;
    size_t bytes_lidos;

    long inicio_blocos_dados = sizeof(Disco) + (sb.qtd_blocos * sizeof(int)) + (sb.qtd_inodes * sizeof(inode));
    while ((bytes_lidos = fread(buffer, 1, sb.tamanho_bloco, arquivo_real)) > 0) {

        int id_bloco = alocar_bloco(nome_disco); 
        if (id_bloco == -1) {
            printf("Erro: Disco virtual cheio no meio da copia!\n");
            break;
        }

        meu_arquivo.blocos[blocos_usados] = id_bloco;
        blocos_usados++;

        long pos_exata = inicio_blocos_dados + (id_bloco * sb.tamanho_bloco);
        f_disco = fopen(nome_disco, "r+b");
        fseek(f_disco, pos_exata, SEEK_SET);

        fwrite(buffer, 1, bytes_lidos, f_disco);
        fclose(f_disco);
    }

    meu_arquivo.modificado = time(NULL);
    guardar_inode(nome_disco, id_inode, &meu_arquivo);

    free(buffer);
    fclose(arquivo_real);
    
    return id_inode;
}   

void exibir_conteudo_arquivo(const char *nome_disco, int id_inode){
    inode in;
    ler_inode(nome_disco, id_inode, &in);
    if (in.is_diretorio == 1) {
        printf("Erro: '%s' e um diretorio, nao um arquivo.\n", in.nome);
        return;
    }
    in.acessado = time(NULL);
    guardar_inode(nome_disco, id_inode, &in);
    FILE *f = fopen(nome_disco, "rb");
    Disco sb;
    fread(&sb, sizeof(Disco), 1, f);
    long inicio_blocos_dados = sizeof(Disco) + (sb.qtd_blocos * sizeof(int)) + (sb.qtd_inodes * sizeof(inode));
    int bytes_restantes = in.tamanho;
    char *buffer = (char *)malloc(sb.tamanho_bloco+1);
    for (int i = 0; i < 12; i++) {
        if (in.blocos[i] == -1) break;

        int bytes_para_ler = (bytes_restantes > sb.tamanho_bloco) ? sb.tamanho_bloco : bytes_restantes;

        long pos_exata = inicio_blocos_dados + (in.blocos[i] * sb.tamanho_bloco);
        fseek(f, pos_exata, SEEK_SET);
        
        fread(buffer, 1, bytes_para_ler, f);
        buffer[bytes_para_ler] = '\0'; 

        printf("%s", buffer);
        bytes_restantes -= bytes_para_ler;

        if (bytes_restantes <= 0) break;
    }
    printf("\n-----------------------\n");
    free(buffer);
    fclose(f);
}

int remover_arquivo(const char *nome_disco, const char *nome_arquivo, int id_pai){
    int id_filho = buscar_filho_por_nome(nome_disco, id_pai, nome_arquivo);
    if (id_filho == -1) {
        printf("Erro: Arquivo '%s' nao encontrado.\n", nome_arquivo);
        return -1;
    }
    inode in;
    ler_inode(nome_disco, id_filho, &in);
    if (in.is_diretorio == 1) {
        printf("Erro: '%s' e um diretorio, nao um arquivo.\n", nome_arquivo);
        return -1;
    }
    for (int i = 0; i < 12; i++) {
        if (in.blocos[i] != -1) {
            liberar_bloco(nome_disco, in.blocos[i]);
        }
    }
    inode pai;
    ler_inode(nome_disco, id_pai, &pai);
    if(pai.id_primeiroFilho == id_filho){
        pai.id_primeiroFilho = in.id_proximoIrmao;
    } else {
        int id_irmao = pai.id_primeiroFilho;
        inode irmao;
        while (id_irmao != -1) {
            ler_inode(nome_disco, id_irmao, &irmao);
            if (irmao.id_proximoIrmao == id_filho) {
                irmao.id_proximoIrmao = in.id_proximoIrmao;
                guardar_inode(nome_disco, irmao.id, &irmao);
                break;
            }
            id_irmao = irmao.id_proximoIrmao;
        }
    }liberar_inode(nome_disco, id_filho);
    return 0;
}

int remover_diretorio(const char *nome_disco, const char *nome_pasta, int id_pai) {
    int id_alvo = buscar_filho_por_nome(nome_disco, id_pai, nome_pasta);

    if (id_alvo == -1) {
        printf("Erro: Diretorio '%s' nao encontrado.\n", nome_pasta);
        return -1;
    }

    inode alvo;
    ler_inode(nome_disco, id_alvo, &alvo);

    if (alvo.is_diretorio == 0) {
        printf("Erro: '%s' e um arquivo. Use o comando rm.\n", nome_pasta);
        return -1;
    }

    if (alvo.id_primeiroFilho != -1) {
        printf("Erro: O diretorio '%s' nao esta vazio.\n", nome_pasta);
        return -1;
    }

    for (int i = 0; i < 12; i++) {
        if (alvo.blocos[i] != -1) {
            liberar_bloco(nome_disco, alvo.blocos[i]);
        }
    }

    inode pai;
    ler_inode(nome_disco, id_pai, &pai);

    if (pai.id_primeiroFilho == id_alvo) {
        pai.id_primeiroFilho = alvo.id_proximoIrmao;
        pai.modificado = time(NULL);
        guardar_inode(nome_disco, id_pai, &pai);
    } else {
        int id_atual = pai.id_primeiroFilho;
        inode atual;

        while (id_atual != -1) {
            ler_inode(nome_disco, id_atual, &atual);

            if (atual.id_proximoIrmao == id_alvo) {
                atual.id_proximoIrmao = alvo.id_proximoIrmao;
                guardar_inode(nome_disco, atual.id, &atual);
                break;
            }
            id_atual = atual.id_proximoIrmao;
        }
    }

    liberar_inode(nome_disco, id_alvo);

    return 0;
}

int renomear_item(const char *nome_disco, const char *nome_antigo, const char *nome_novo, int id_pai){
    int id_item = buscar_filho_por_nome(nome_disco, id_pai, nome_antigo);
    if (id_item == -1) {
        printf("Erro: Item '%s' nao encontrado.\n", nome_antigo);
        return -1;
    }
    if (buscar_filho_por_nome(nome_disco, id_pai, nome_novo) != -1) {
        printf("Erro: Ja existe um item com o nome '%s'.\n", nome_novo);
        return -1;
    }
    inode item;
    ler_inode(nome_disco, id_item, &item);
    strncpy(item.nome, nome_novo, sizeof(item.nome) - 1);
    item.nome[sizeof(item.nome) - 1] = '\0';
    item.modificado = time(NULL);
    guardar_inode(nome_disco, id_item, &item);
    return 0;
}

int mover_item(const char *nome_disco, const char *nome_item, const char *nome_destino, int id_pai_atual) {
    int id_item = buscar_filho_por_nome(nome_disco, id_pai_atual, nome_item);
    
    if (id_item == -1) {
        printf("Erro: Item '%s' nao encontrado.\n", nome_item);
        return -1;
    }
    
    int id_novo_pai = buscar_filho_por_nome(nome_disco, id_pai_atual, nome_destino);
    
    if (id_novo_pai == -1) {
        printf("Erro: Diretorio de destino '%s' nao encontrado.\n", nome_destino);
        return -1;
    }

    inode novo_pai;
    ler_inode(nome_disco, id_novo_pai, &novo_pai);

    if (novo_pai.is_diretorio == 0) {
        printf("Erro: Destino '%s' nao e um diretorio.\n", nome_destino);
        return -1;
    }
    
    if (buscar_filho_por_nome(nome_disco, id_novo_pai, nome_item) != -1) {
        printf("Erro: Item '%s' ja existe no destino.\n", nome_item);
        return -1;
    }
    
    inode pai_atual;
    ler_inode(nome_disco, id_pai_atual, &pai_atual);
    inode item;
    ler_inode(nome_disco, id_item, &item);

    if (pai_atual.id_primeiroFilho == id_item) {
        pai_atual.id_primeiroFilho = item.id_proximoIrmao;
    } else {
        int id_irmao = pai_atual.id_primeiroFilho;
        inode irmao;
        while (id_irmao != -1) {
            ler_inode(nome_disco, id_irmao, &irmao);
            if (irmao.id_proximoIrmao == id_item) {
                irmao.id_proximoIrmao = item.id_proximoIrmao;
                guardar_inode(nome_disco, irmao.id, &irmao);
                break;
            }
            id_irmao = irmao.id_proximoIrmao;
        }
    }
    
    guardar_inode(nome_disco, id_pai_atual, &pai_atual);
    
    item.id_pai = id_novo_pai;
    item.id_proximoIrmao = -1;

    if (novo_pai.id_primeiroFilho == -1) {
        novo_pai.id_primeiroFilho = id_item;
    } else {
        int id_irmao = novo_pai.id_primeiroFilho;
        inode irmao;
        while (1) {
            ler_inode(nome_disco, id_irmao, &irmao);
            if (irmao.id_proximoIrmao == -1) {
                irmao.id_proximoIrmao = id_item;
                guardar_inode(nome_disco, irmao.id, &irmao);
                break;
            }
            id_irmao = irmao.id_proximoIrmao;
        }
    }
    
    novo_pai.modificado = time(NULL);
    guardar_inode(nome_disco, id_novo_pai, &novo_pai);
    
    item.modificado = time(NULL);
    guardar_inode(nome_disco, id_item, &item);

    return 0;
}