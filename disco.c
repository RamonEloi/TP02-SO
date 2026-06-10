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