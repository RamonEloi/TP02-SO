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
        printf("Comandos disponiveis: mkdir <nome>, touch <nome>, cd <diretorio>,pwd, importar <arquivo_real> <nome_virtual>,\n"
            "cat <nome>, rm <nome>, rmdir <nome>,renomear <nome_antigo> <nome_novo>,\n" 
            "mv <nome_item> <nome_destino>, ls, sair\n\n");

    char comando[256];
    int diretorio_atual_id = 0; 
    char caminho_atual[256];

    while(1) {
        obter_caminho_atual("meu_hd_virtual.bin",diretorio_atual_id, caminho_atual);

        printf("meu-disco:%s> ", caminho_atual); 
        
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
        else if(strncmp(comando,"cd ",3)==0){
            char *destino = comando + 3;

            mudar_diretorio("meu_hd_virtual.bin",destino, &diretorio_atual_id);
        }
        else if(strcmp(comando,"pwd")==0){ 
            obter_caminho_atual("meu_hd_virtual.bin", diretorio_atual_id, caminho_atual);
            printf("%s\n", caminho_atual);
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
        else if (strncmp(comando, "rm ", 3) == 0) {
            char *nome_arq = comando + 3;
            int resultado = remover_arquivo("meu_hd_virtual.bin", nome_arq, diretorio_atual_id);
            if (resultado != -1) {
                printf("Arquivo apagado com sucesso!\n");
            }
        }
        else if (strncmp(comando, "rmdir ", 6) == 0) {
            char *nome_pasta = comando + 6;
            int resultado = remover_diretorio("meu_hd_virtual.bin", nome_pasta, diretorio_atual_id);
            if (resultado != -1) {
                printf("Diretorio apagado com sucesso!\n");
            }
        }
        else if (strncmp(comando, "renomear ", 9) == 0) {
            char nome_antigo[100], nome_novo[100];
            if (sscanf(comando + 9, "%s %s", nome_antigo, nome_novo) == 2) {
                int resultado = renomear_item("meu_hd_virtual.bin", nome_antigo, nome_novo, diretorio_atual_id);
                if (resultado != -1) {
                    printf("Item renomeado com sucesso!\n");
                }
            } else {
                printf("Uso correto: renomear <nome_antigo> <nome_novo>\n");   
            }
        }
        else if (strncmp(comando, "mv ", 3) == 0) {
            char nome_item[100], nome_destino[100];
            if (sscanf(comando + 3, "%s %s", nome_item, nome_destino) == 2) {
                int resultado = mover_item("meu_hd_virtual.bin", nome_item, nome_destino, diretorio_atual_id);
                if (resultado != -1) {
                    printf("Item movido com sucesso!\n");
                }
            } else {
                printf("Uso correto: mv <nome_arquivo> <nome_pasta_destino>\n");   
            }
        }
        else if (strlen(comando) > 0) {
            printf("Erro: Comando não reconhecido.\n");
        }
    }

    return 0;
}