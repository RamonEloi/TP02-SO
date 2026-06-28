#include "common.h"
#include "disco.h"
#include "diretorio.h"
#include "navegacao.h"
#include "arquivo.h"
#include "menu.h"

int main(int argc, char *argv[]) {
    int tamanho_disco;
    int tamanho_bloco;
    int max_inodes;
    FILE *stream_entrada = stdin;

    if(argc > 1){
        stream_entrada = fopen(argv[1],"r");
        if(stream_entrada == NULL){
            printf("Erro ao abrir arquivo de comandos.\n");
            stream_entrada = stdin;
        }else{
            printf("Arquivo de comandos carregado.\n\n");
        }
    }

    printf("\n=================================================\n");
    printf("     CONFIGURACAO DO SISTEMA DE ARQUIVOS\n");
    printf("=================================================\n\n");
    printf("Tamanho do disco (em bytes, ex: 10485760): ");
    if(scanf("%d",&tamanho_disco)!=1)
        tamanho_disco = 10485760;
    printf("Tamanho do bloco (em bytes, ex: 4096): ");
    if(scanf("%d",&tamanho_bloco)!=1)
        tamanho_bloco = 4096;
    printf("Quantidade de i-nodes (ex: 1000): ");
    if(scanf("%d",&max_inodes)!=1)
        max_inodes = 1000;

    int c;
    while((c=getchar())!='\n' && c!=EOF);

    printf("\nMontando sistema de arquivos...\n");
    printf("[OK] Criando superbloco\n");
    printf("[OK] Inicializando i-nodes\n");
    printf("[OK] Criando bitmap\n");

    formatar_disco("meu_hd_virtual.bin", tamanho_disco, tamanho_bloco, max_inodes);
    inicializar_diretorio_raiz("meu_hd_virtual.bin");
    printf("[OK] Diretorio raiz criado\n");

    iniciar_menu_interativo(stream_entrada, tamanho_disco, tamanho_bloco, max_inodes);

    if(stream_entrada != stdin)
        fclose(stream_entrada);
    return 0;
}