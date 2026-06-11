#include "disco.h"   
#include "i-node.h"
// Cria o disco do zero
void formatar_disco(const char* nome_arquivo_real, int tamanho_disco, int tamanho_bloco, int qtd_inodes) {
    FILE *arquivo_disco = fopen(nome_arquivo_real, "wb");
    if (arquivo_disco == NULL) {
        printf("Erro ao criar o disco virtual.\n");
        return;
    }

    Disco superbloco;
    superbloco.tamanho_disco = tamanho_disco;
    superbloco.tamanho_bloco = tamanho_bloco;
    superbloco.qtd_blocos = tamanho_disco / tamanho_bloco;
    superbloco.blocos_livres = superbloco.qtd_blocos;
    superbloco.qtd_inodes = qtd_inodes;
    superbloco.inodes_livres = qtd_inodes;

    fwrite(&superbloco, sizeof(Disco), 1, arquivo_disco);

    // Mapa de bits (marcando 0 = livre, 1 = ocupado)
    int *bitmap_blocos = (int*) calloc(superbloco.qtd_blocos, sizeof(int));
    fwrite(bitmap_blocos, sizeof(int), superbloco.qtd_blocos, arquivo_disco);
    free(bitmap_blocos);

    inode inode_vazio;
    memset(&inode_vazio, 0, sizeof(inode));
    inode_vazio.id = -1; 

    for (int i = 0; i < qtd_inodes; i++) {
        fwrite(&inode_vazio, sizeof(inode), 1, arquivo_disco);
    }

    char *bloco_vazio = (char*) calloc(tamanho_bloco, sizeof(char));
    for (int i = 0; i < superbloco.qtd_blocos; i++) {
        fwrite(bloco_vazio, sizeof(char), tamanho_bloco, arquivo_disco);
    }
    free(bloco_vazio);

    fclose(arquivo_disco);
    printf("Disco '%s' formatado com sucesso!\n", nome_arquivo_real);
    printf("Tamanho: %d bytes | Blocos: %d de %d bytes | Inodes permitidos: %d\n", 
           tamanho_disco, superbloco.qtd_blocos, tamanho_bloco, qtd_inodes);
}

int alocar_bloco(const char* nome_disco) {
    FILE *f = fopen(nome_disco, "r+b");
    if (f == NULL) return -1;

    Disco sb;
    fread(&sb, sizeof(Disco), 1, f);

    // Se não houver blocos livres, nem lê o bitmap
    if (sb.blocos_livres <= 0) {
        fclose(f);
        return -1;
    }

    // Carrega o bitmap para a memória
    int *bitmap = (int*) malloc(sb.qtd_blocos * sizeof(int));
    fseek(f, sizeof(Disco), SEEK_SET);
    fread(bitmap, sizeof(int), sb.qtd_blocos, f);

    int bloco_encontrado = -1;
    for (int i = 0; i < sb.qtd_blocos; i++) {
        if (bitmap[i] == 0) { // 0 significa LIVRE
            bitmap[i] = 1;    // Marca como OCUPADO
            bloco_encontrado = i;
            break;
        }
    }

    if (bloco_encontrado != -1) {
        sb.blocos_livres--;

        // Salva o bitmap atualizado de volta no disco
        fseek(f, sizeof(Disco), SEEK_SET);
        fwrite(bitmap, sizeof(int), sb.qtd_blocos, f);

        // Salva o superbloco atualizado no início do disco
        fseek(f, 0, SEEK_SET);
        fwrite(&sb, sizeof(Disco), 1, f);
    }

    free(bitmap);
    fclose(f);
    return bloco_encontrado;
}

/**
 * Libera um bloco no bitmap (volta para 0), limpa o conteúdo dele com zeros
 * (boa prática) e incrementa os blocos livres no Superbloco.
 */
void liberar_bloco(const char* nome_disco, int bloco_id) {
    FILE *f = fopen(nome_disco, "r+b");
    if (f == NULL) return;

    Disco sb;
    fread(&sb, sizeof(Disco), 1, f);

    if (bloco_id < 0 || bloco_id >= sb.qtd_blocos) {
        fclose(f);
        return;
    }

    // 1. Atualiza a posição exata daquele bloco no bitmap para 0
    long pos_bitmap = sizeof(Disco) + (bloco_id * sizeof(int));
    int livre = 0;
    fseek(f, pos_bitmap, SEEK_SET);
    fwrite(&livre, sizeof(int), 1, f);

    // 2. Limpa o bloco de dados real preenchendo-o com zeros (limpeza física)
    long inicio_blocos_dados = sizeof(Disco) + (sb.qtd_blocos * sizeof(int)) + (sb.qtd_inodes * sizeof(inode));
    long pos_bloco_dados = inicio_blocos_dados + (bloco_id * sb.tamanho_bloco);
    
    char *bloco_vazio = (char*) calloc(sb.tamanho_bloco, sizeof(char));
    fseek(f, pos_bloco_dados, SEEK_SET);
    fwrite(bloco_vazio, sizeof(char), sb.tamanho_bloco, f);
    free(bloco_vazio);

    // 3. Atualiza os metadados do Superbloco
    sb.blocos_livres++;
    fseek(f, 0, SEEK_SET);
    fwrite(&sb, sizeof(Disco), 1, f);

    fclose(f);
}

/**
 * Varre a tabela de i-nodes procurando por um registro com id == -1 (livre).
 * Retorna o ID do i-node alocado ou -1 se não houver i-nodes livres.
 */
int alocar_inode(const char* nome_disco) {
    FILE *f = fopen(nome_disco, "r+b");
    if (f == NULL) return -1;

    Disco sb;
    fread(&sb, sizeof(Disco), 1, f);

    if (sb.inodes_livres <= 0) {
        fclose(f);
        return -1;
    }

    long inicio_inodes = sizeof(Disco) + (sb.qtd_blocos * sizeof(int));
    fseek(f, inicio_inodes, SEEK_SET);

    int inode_encontrado = -1;
    inode in;

    // Varre a tabela de i-nodes um por um
    for (int i = 0; i < sb.qtd_inodes; i++) {
        fread(&in, sizeof(inode), 1, f);
        if (in.id == -1) { // Encontrou um i-node livre
            inode_encontrado = i;
            break;
        }
    }

    if (inode_encontrado != -1) {
        sb.inodes_livres--;
        
        // Atualiza o Superbloco
        fseek(f, 0, SEEK_SET);
        fwrite(&sb, sizeof(Disco), 1, f);
    }

    fclose(f);
    return inode_encontrado;
}

/**
 * Sobrescreve o i-node especificado com uma estrutura vazia (id = -1)
 * e incrementa os i-nodes livres no Superbloco.
 */
void liberar_inode(const char* nome_disco, int id_inode) {
    FILE *f = fopen(nome_disco, "r+b");
    if (f == NULL) return;

    Disco sb;
    fread(&sb, sizeof(Disco), 1, f);

    if (id_inode < 0 || id_inode >= sb.qtd_inodes) {
        fclose(f);
        return;
    }

    // Cria um molde de i-node limpo
    inode inode_vazio;
    memset(&inode_vazio, 0, sizeof(inode));
    inode_vazio.id = -1;

    // Grava por cima do i-node antigo
    long inicio_inodes = sizeof(Disco) + (sb.qtd_blocos * sizeof(int));
    long pos_exata = inicio_inodes + (id_inode * sizeof(inode));
    fseek(f, pos_exata, SEEK_SET);
    fwrite(&inode_vazio, sizeof(inode), 1, f);

    // Atualiza o Superbloco
    sb.inodes_livres++;
    fseek(f, 0, SEEK_SET);
    fwrite(&sb, sizeof(Disco), 1, f);

    fclose(f);
}