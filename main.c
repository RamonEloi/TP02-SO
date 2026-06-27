#include "common.h"
#include "disco.h"
#include "diretorio.h"
#include "navegacao.h"
#include "arquivo.h"

int main(int argc, char *argv[]) {
    int tamanho_disco; 
    int tamanho_bloco;
    int max_inodes;
    int modo_verboso = 0; // 0 = falso, 1 = verdadeiro
    FILE *stream_entrada = stdin; // Por padrão, lê do teclado

    // --- Verificação do Modo de Execução (Arquivo de Entrada) ---
    if (argc > 1) {
        stream_entrada = fopen(argv[1], "r");
        if (stream_entrada == NULL) {
            printf("Erro: Nao foi possivel abrir o arquivo de comandos '%s'.\n", argv[1]);
            printf("Mudando automaticamente para o Modo Interativo (teclado).\n\n");
            stream_entrada = stdin;
        } else {
            printf("Arquivo de comandos '%s' carregado com sucesso!\n", argv[1]);
            printf("O simulador executara as operacoes em lote.\n\n");
        }
    }

    // --- Configuração Dinâmica do Disco ---
    // Nota: A configuração inicial sempre será feita via teclado (stdin)
    printf("--- Inicializacao do Sistema de Arquivos ---\n");
    
    printf("Digite o tamanho do disco virtual (em bytes, ex: 10485760): ");
    if (scanf("%d", &tamanho_disco) != 1) {
        tamanho_disco = 10485760;
    }

    printf("Digite o tamanho do bloco (em bytes, ex: 4096): ");
    if (scanf("%d", &tamanho_bloco) != 1) {
        tamanho_bloco = 4096;
    }

    printf("Digite a quantidade maxima de i-nodes (ex: 1000): ");
    if (scanf("%d", &max_inodes) != 1) {
        max_inodes = 1000;
    }
    
    // Limpar o buffer do teclado para o fgets subsequente
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    printf("\nFormatando e inicializando o disco virtual...\n");
    formatar_disco("meu_hd_virtual.bin", tamanho_disco, tamanho_bloco, max_inodes);
    inicializar_diretorio_raiz("meu_hd_virtual.bin");

    printf("\n--- Simulador de Sistema de Arquivos ---\n");
    printf("Comandos disponiveis:\n"
           "  mkdir <nome>, touch <nome>, cd <diretorio>, pwd, ls\n"
           "  importar <arquivo_real> <nome_virtual>, cat <nome>, rm <nome>, rmdir <nome>\n"
           "  renomear <nome_antigo> <nome_novo>, mv <nome_item> <nome_destino>\n"
           "  verbose, sair\n\n");

    char comando[256];
    int diretorio_atual_id = 0; 
    char caminho_atual[256];

    while(1) {
        obter_caminho_atual("meu_hd_virtual.bin", diretorio_atual_id, caminho_atual);

        // Exibe o prompt na tela
        if (modo_verboso) {
            printf("[VERBOSO] meu-disco:%s> ", caminho_atual);
        } else {
            printf("meu-disco:%s> ", caminho_atual);
        }
        
        // Lê do teclado (stdin) ou do arquivo carregado (stream_entrada)
        if (fgets(comando, sizeof(comando), stream_entrada) == NULL) {
            if (stream_entrada != stdin) {
                printf("\n[Fim do arquivo de comandos alcancado. Mudando para modo interativo...]\n");
                fclose(stream_entrada);
                stream_entrada = stdin;
                continue; // Volta para o loop esperando comandos do teclado
            }
            break; 
        }

        // Se estiver lendo de um arquivo, mostra o comando na tela para o usuário acompanhar
        if (stream_entrada != stdin) {
            printf("%s", comando);
            // Caso a linha do arquivo não termine com '\n' (última linha), quebra a linha manualmente
            if (comando[strlen(comando) - 1] != '\n') {
                printf("\n");
            }
        }

        // Remove quebras de linha tanto do Linux (\n) quanto do Windows (\r\n)
        comando[strcspn(comando, "\n\r")] = 0; 

        // Ignora linhas vazias ou comentários (linhas que começam com #) no arquivo
        if (strlen(comando) == 0 || comando[0] == '#') {
            continue;
        }

        // --- Processamento dos Comandos ---
        if (strcmp(comando, "verbose") == 0) {
            modo_verboso = !modo_verboso;
            printf("Modo verboso %s!\n", modo_verboso ? "ATIVADO" : "DESATIVADO");
        }
        else if (strncmp(comando, "mkdir ", 6) == 0) {
            char *nome_pasta = comando + 6; 
            
            if (modo_verboso) {
                printf("[DEBUG] Executando 'mkdir'. Solicitando alocacao de entrada de diretorio para '%s' dentro do ID pai %d.\n", nome_pasta, diretorio_atual_id);
            }

            int resultado = simular_mkdir("meu_hd_virtual.bin", nome_pasta, diretorio_atual_id);
            if (resultado != -1) {
                printf("Diretorio '%s' criado com sucesso!\n", nome_pasta);
                if (modo_verboso) {
                    printf("[DEBUG] Sucesso: Novo diretorio associado ao i-node: %d.\n", resultado);
                }
            }
        } 
        else if (strncmp(comando, "touch ", 6) == 0){
            char *nome_arquivo = comando + 6;

            if (modo_verboso) {
                printf("[DEBUG] Executando 'touch'. Criando estrutura de arquivo vazio '%s' no diretorio ID %d.\n", nome_arquivo, diretorio_atual_id);
            }

            int resultado = criar_arquivo("meu_hd_virtual.bin", nome_arquivo, diretorio_atual_id);
            if (resultado != -1) {
                printf("Arquivo '%s' criado com sucesso!\n", nome_arquivo);
                if (modo_verboso) {
                    printf("[DEBUG] Sucesso: Arquivo criado e i-node %d reservado.\n", resultado);
                }
            }
        }
        else if(strncmp(comando, "cd ", 3) == 0){
            char *destino = comando + 3;

            if (modo_verboso) {
                printf("[DEBUG] Executando 'cd'. Buscando subdiretorio '%s' a partir do ID %d.\n", destino, diretorio_atual_id);
            }

            mudar_diretorio("meu_hd_virtual.bin", destino, &diretorio_atual_id);
        }
        else if(strcmp(comando, "pwd") == 0){ 
            obter_caminho_atual("meu_hd_virtual.bin", diretorio_atual_id, caminho_atual);
            printf("%s\n", caminho_atual);
        }
        else if (strcmp(comando, "ls") == 0) {
            if (modo_verboso) {
                printf("[DEBUG] Executando 'ls'. Lendo bloco de dados do i-node %d para listar entradas.\n", diretorio_atual_id);
            }
            listar_diretorio("meu_hd_virtual.bin", diretorio_atual_id);
        }
        else if (strcmp(comando, "sair") == 0) {
            printf("Encerrando...\n");
            break; 
        }
        else if (strncmp(comando, "importar ", 9) == 0) {
            char nome_real[100], nome_virt[100];
            if (sscanf(comando + 9, "%s %s", nome_real, nome_virt) == 2) {
                if (modo_verboso) {
                    printf("[DEBUG] Executando 'importar'. Abrindo '%s' e fragmentando em blocos de %d bytes.\n", nome_real, tamanho_bloco);
                }

                int resultado = importar_arquivo_real("meu_hd_virtual.bin", nome_real, nome_virt, diretorio_atual_id);
                if (resultado != -1) {
                    printf("Arquivo importado com sucesso!\n");
                    if (modo_verboso) {
                        printf("[DEBUG] Sucesso: Dados gravados e i-node %d atualizado.\n", resultado);
                    }
                }
            } else {
                printf("Uso correto: importar <arquivo_real> <nome_virtual>\n");   
            }
        }
        else if (strncmp(comando, "cat ", 4) == 0) {
            char *nome_arq = comando + 4;
            
            if (modo_verboso) {
                printf("[DEBUG] Executando 'cat'. Localizando i-node para o arquivo '%s'.\n", nome_arq);
            }

            int id_alvo = buscar_filho_por_nome("meu_hd_virtual.bin", diretorio_atual_id, nome_arq);
            if (id_alvo != -1) {
                if (modo_verboso) {
                    printf("[DEBUG] Arquivo encontrado no i-node %d. Lendo blocos de dados.\n", id_alvo);
                }
                exibir_conteudo_arquivo("meu_hd_virtual.bin", id_alvo);
            } else {
                printf("Arquivo nao encontrado.\n");
            }
        }
        else if (strncmp(comando, "rm ", 3) == 0) {
            char *nome_arq = comando + 3;

            if (modo_verboso) {
                printf("[DEBUG] Executando 'rm'. Removendo '%s' e liberando blocos associados.\n", nome_arq);
            }

            int resultado = remover_arquivo("meu_hd_virtual.bin", nome_arq, diretorio_atual_id);
            if (resultado != -1) {
                printf("Arquivo apagado com sucesso!\n");
            }
        }
        else if (strncmp(comando, "rmdir ", 6) == 0) {
            char *nome_pasta = comando + 6;

            if (modo_verboso) {
                printf("[DEBUG] Executando 'rmdir'. Verificando se o diretorio '%s' esta vazio.\n", nome_pasta);
            }

            int resultado = remover_diretorio("meu_hd_virtual.bin", nome_pasta, diretorio_atual_id);
            if (resultado != -1) {
                printf("Diretorio apagado com sucesso!\n");
            }
        }
        else if (strncmp(comando, "renomear ", 9) == 0) {
            char nome_antigo[100], nome_novo[100];
            if (sscanf(comando + 9, "%s %s", nome_antigo, nome_novo) == 2) {
                if (modo_verboso) {
                    printf("[DEBUG] Executando 'renomear'. Mudando de '%s' para '%s'.\n", nome_antigo, nome_novo);
                }

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
                if (modo_verboso) {
                    printf("[DEBUG] Executando 'mv'. Modificando o pai de '%s' para o ID de '%s'.\n", nome_item, nome_destino);
                }

                int resultado = mover_item("meu_hd_virtual.bin", nome_item, nome_destino, diretorio_atual_id);
                if (resultado != -1) {
                    printf("Item movido com sucesso!\n");
                }
            } else {
                printf("Uso correto: mv <nome_arquivo> <nome_pasta_destino>\n");   
            }
        }
        else if (strlen(comando) > 0) {
            printf("Erro: Comando nao reconhecido.\n");
        }
    }

    if (stream_entrada != stdin) {
        fclose(stream_entrada);
    }

    return 0;
}