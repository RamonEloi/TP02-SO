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
   
    return 0;
}