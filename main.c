#include "common.h"
#include "disco.h"
#include "diretorio.h"

 //Cruzeiro maior de Minas

int main(){
    // Parâmetros para o teste:
    // Disco de 10 MB (10 * 1024 * 1024 bytes)
    // Blocos de 4 KB (4096 bytes)
    // Limite de 1000 i-nodes (arquivos/diretórios)
    int tamanho_disco = 10485760; 
    int tamanho_bloco = 4096;
    int max_inodes = 1000;

    formatar_disco("meu_hd_virtual.bin", tamanho_disco, tamanho_bloco, max_inodes);
    inicializar_diretorio_raiz("meu_hd_virtual.bin");
    printf("\n--- Testando Alocação ---\n");
    int b1 = alocar_bloco("meu_hd_virtual.bin");
    int b2 = alocar_bloco("meu_hd_virtual.bin");
    int i1 = alocar_inode("meu_hd_virtual.bin");

    printf("Bloco 1 alocado: %d (Esperado: 1, pois a raiz usa o 0)\n", b1);
    printf("Bloco 2 alocado: %d (Esperado: 2)\n", b2);
    printf("i-node alocado: %d (Esperado: 1, pois a raiz usa o 0)\n", i1);

    // Liberando para ver se volta a funcionar
    liberar_bloco("meu_hd_virtual.bin", b1);
    int b3 = alocar_bloco("meu_hd_virtual.bin");
    printf("Bloco alocado após liberação: %d (Esperado: 1, pois foi reciclado)\n", b3);
    return 0;
}