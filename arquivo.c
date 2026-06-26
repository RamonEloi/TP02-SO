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