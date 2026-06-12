#include "i-node.h"
#include "disco.h"

void guardar_inode(const char* nome_disco, int id_inode, inode* in) {
    FILE *f = fopen(nome_disco, "r+b");
    if (f == NULL) return;

    Disco sb;
    fread(&sb, sizeof(Disco), 1, f);

    //Calcular onde começam os i-nodes
    // Salta o Superbloco + Salta o Mapa de Bits
    long inicio_inodes = sizeof(Disco) + (sb.qtd_blocos * sizeof(int));
    
    long posicao_exata = inicio_inodes + (id_inode * sizeof(inode));

    fseek(f, posicao_exata, SEEK_SET);
    fwrite(in, sizeof(inode), 1, f);

    fclose(f);
}

// Função para ler um i-node do disco para a memória
void ler_inode(const char* nome_disco, int id_inode, inode* in) {
    FILE *f = fopen(nome_disco, "rb");
    if (f == NULL) return;

    Disco sb;
    fread(&sb, sizeof(Disco), 1, f);

    long inicio_inodes = sizeof(Disco) + (sb.qtd_blocos * sizeof(int));
    long posicao_exata = inicio_inodes + (id_inode * sizeof(inode));

    fseek(f, posicao_exata, SEEK_SET);
    fread(in, sizeof(inode), 1, f);

    fclose(f);
}