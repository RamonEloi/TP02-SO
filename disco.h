#include "common.h"

typedef struct {
    int tamanho_disco;
    int tamanho_bloco;
    int qtd_blocos;
    int blocos_livres;
    int qtd_inodes;        // Quantidade total de i-nodes permitidos
    int inodes_livres;     // Quantos i-nodes ainda podem ser criados
} Disco;

void formatar_disco(const char* nome_arquivo_real, int tamanho_disco, int tamanho_bloco, int qtd_inodes);
