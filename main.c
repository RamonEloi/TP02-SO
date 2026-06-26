#include "common.h"
#include "disco.h"
#include "diretorio.h"
#include "navegacao.h"
#include "arquivo.h"

int main(){
    int tamanho_disco = 10485760; 
    int tamanho_bloco = 4096;
    int max_inodes = 1000;

    formatar_disco("meu_hd_virtual.bin", tamanho_disco, tamanho_bloco, max_inodes);
    inicializar_diretorio_raiz("meu_hd_virtual.bin");

    printf("\n--- Simulador de Sistema de Arquivos (Modo Simples) ---\n");
    printf("Comandos disponiveis: mkdir <nome>, touch <nome>, importar <arquivo_real> <nome_virtual>, cat <nome>, ls, sair\n\n");

    char comando[256];
    int diretorio_atual_id = 0; 

    while(1) {
        printf("meu-disco> "); 
        
        if (fgets(comando, sizeof(comando), stdin) == NULL) break;

        comando[strcspn(comando, "\n")] = 0; 

        if (strncmp(comando, "mkdir ", 6) == 0) {
            char *nome_pasta = comando + 6; 
            
            int resultado = simular_mkdir("meu_hd_virtual.bin", nome_pasta, diretorio_atual_id);

            if (resultado != -1) {
                printf("Diretorio '%s' criado com sucesso!\n", nome_pasta);
            }
        } 
        else if (strncmp(comando, "touch ", 6) == 0){
            char *nome_arquivo = comando + 6;
            int resultado = criar_arquivo("meu_hd_virtual.bin", nome_arquivo, diretorio_atual_id);
            
            if (resultado != -1) {
                printf("Arquivo '%s' criado com sucesso!\n", nome_arquivo);
            }
        }
        else if (strcmp(comando, "ls") == 0) {
            listar_diretorio("meu_hd_virtual.bin", diretorio_atual_id);
        }
        else if (strcmp(comando, "sair") == 0) {
            printf("Encerrando...\n");
            break; 
        }
        else if (strncmp(comando, "importar ", 9) == 0) {
            char nome_real[100], nome_virt[100];
            if (sscanf(comando + 9, "%s %s", nome_real, nome_virt) == 2) {
                int resultado = importar_arquivo_real("meu_hd_virtual.bin", nome_real, nome_virt, diretorio_atual_id);
                if (resultado != -1) {
                    printf("Arquivo importado com sucesso!\n");
                }
            } else {
                printf("Uso correto: importar <arquivo_real> <nome_virtual>\n");   
            }
        }
        else if (strncmp(comando, "cat ", 4) == 0) {
            char *nome_arq = comando + 4;
            int id_alvo = buscar_filho_por_nome("meu_hd_virtual.bin", diretorio_atual_id, nome_arq);
            if (id_alvo != -1) {
                exibir_conteudo_arquivo("meu_hd_virtual.bin", id_alvo);
            } else {
                printf("Arquivo nao encontrado.\n");
            }
        }
        else if (strlen(comando) > 0) {
            printf("Erro: Comando não reconhecido.\n");
        }
    }

    return 0;
}