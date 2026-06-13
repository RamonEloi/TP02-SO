#include "common.h"
#include "disco.h"
#include "diretorio.h"
#include "navegacao.h"
#include "arquivo.h"


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

    /*TESTES ANTIGOS (Comentados)
    printf("\n--- Testando Alocacao ---\n");
    int b1 = alocar_bloco("meu_hd_virtual.bin");
    int b2 = alocar_bloco("meu_hd_virtual.bin");
    int i1 = alocar_inode("meu_hd_virtual.bin");

    printf("Bloco 1 alocado: %d (Esperado: 1, pois a raiz usa o 0)\n", b1);
    printf("Bloco 2 alocado: %d (Esperado: 2)\n", b2);
    printf("i-node alocado: %d (Esperado: 1, pois a raiz usa o 0)\n", i1);

    liberar_bloco("meu_hd_virtual.bin", b1);
    int b3 = alocar_bloco("meu_hd_virtual.bin");
    printf("Bloco alocado apos liberacao: %d (Esperado: 1, pois foi reciclado)\n", b3);
    ---------------------------------------------------------------------- */

   //Interface nova
    printf("\n--- Simulador de Sistema de Arquivos (Modo Simples) ---\n");
    printf("Comandos disponiveis: mkdir <nome>, ls, sair\n\n");

    char comando[256];
    int diretorio_atual_id = 0; // Sempre começamos na raiz (i-node 0)

    while(1) {
        printf("meu-disco> "); // O prompt que aparece na tela
        
        // Lê o que você digitar no terminal
        if (fgets(comando, sizeof(comando), stdin) == NULL) break;

        // Limpa o caractere de "Enter" do final da linha
        comando[strcspn(comando, "\n")] = 0; 

        // Se o comando começar com "mkdir "
        if (strncmp(comando, "mkdir ", 6) == 0) {
            char *nome_pasta = comando + 6; // Pega só a palavra depois do espaço
            simular_mkdir("meu_hd_virtual.bin", nome_pasta, diretorio_atual_id);
            printf("Sucesso: Pasta '%s' criada!\n", nome_pasta);
        } 
        
        else if (strncmp(comando, "touch ", 6) == 0){
            char *nome_arquivo = comando + 6;
            criar_arquivo("meu_hd_virtual.bin", nome_arquivo, diretorio_atual_id);
            printf("Arquivo '%s' criado!\n", nome_arquivo);
        }
        // Se o comando for "ls"
        else if (strcmp(comando, "ls") == 0) {
            listar_diretorio("meu_hd_virtual.bin", diretorio_atual_id);
        }
        // Se o comando for "sair"
        else if (strcmp(comando, "sair") == 0) {
            printf("Encerrando...\n");
            break; // Sai do laço while e termina o programa
        }
        // Se o usuário digitar algo diferente (e que não seja só apertar Enter vazio)
        else if (strlen(comando) > 0) {
            printf("Erro: Comando não reconhecido.\n");
        }
    }

    return 0;
}