#include "common.h"
#include "disco.h"
#include "diretorio.h"
#include "navegacao.h"
#include "arquivo.h"

void mostrar_menu(int tamanho_disco, int tamanho_bloco, int max_inodes){
    printf("\n");
    printf("=================================================\n");
    printf("          SIMULADOR DE SISTEMA DE ARQUIVOS        \n");
    printf("=================================================\n\n");
    printf("CONFIGURACAO ATUAL:\n");
    printf("  Tamanho do disco : %d bytes\n", tamanho_disco);
    printf("  Tamanho do bloco : %d bytes\n", tamanho_bloco);
    printf("  Max. blocos      : %d\n", tamanho_disco / tamanho_bloco);
    printf("  Max. i-nodes     : %d\n\n", max_inodes);
    printf("DIRETORIOS:\n");
    printf("  mkdir <nome>                 Criar diretorio\n");
    printf("  rmdir <nome>                 Remover diretorio\n");
    printf("  cd <nome>                    Entrar no diretorio\n");
    printf("  pwd                          Mostrar caminho atual\n");
    printf("  ls                           Listar conteudo\n\n");
    printf("ARQUIVOS:\n");
    printf("  touch <nome>                 Criar arquivo vazio\n");
    printf("  importar <real> <virtual>    Importar arquivo real\n");
    printf("  cat <nome>                   Mostrar conteudo\n");
    printf("  rm <nome>                    Remover arquivo\n");
    printf("  mv <item> <destino>          Mover item\n");
    printf("  renomear <antigo> <novo>     Renomear item\n\n");
    printf("SISTEMA:\n");
    printf("  info                         Informacoes do disco\n");
    printf("  verbose                      Ativar modo detalhado\n");
    printf("  ajuda                        Mostrar menu\n");
    printf("  sair                         Encerrar\n");
    printf("\n=================================================\n\n");
}

int main(int argc, char *argv[]) {
    int tamanho_disco;
    int tamanho_bloco;
    int max_inodes;
    int modo_verboso = 0;
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

    mostrar_menu(tamanho_disco, tamanho_bloco, max_inodes);

    char comando[256];
    int diretorio_atual_id = 0;
    char caminho_atual[256];

    while(1){
        obter_caminho_atual("meu_hd_virtual.bin", diretorio_atual_id, caminho_atual);
        if(modo_verboso)
            printf("\n[VERBOSE][FS] %s $ ", caminho_atual);
        else
            printf("\n[FS] %s $ ", caminho_atual);

        if(fgets(comando,sizeof(comando),stream_entrada)==NULL){
            break;
        }
        comando[strcspn(comando,"\n\r")] = 0;
        if(strlen(comando)==0)
            continue;

        if(strcmp(comando,"ajuda")==0){
            mostrar_menu(tamanho_disco, tamanho_bloco, max_inodes);
        }
        else if(strcmp(comando,"verbose")==0){
            modo_verboso = !modo_verboso;
            printf("Modo verboso %s\n", modo_verboso ? "ativado":"desativado");
        }
        else if(strcmp(comando,"info")==0){
            mostrar_info_disco("meu_hd_virtual.bin");
        }
        else if(strncmp(comando,"mkdir ",6)==0){
            char *nome = comando+6;
            int r = simular_mkdir("meu_hd_virtual.bin", nome, diretorio_atual_id);
            if(r!=-1)
                printf("Diretorio criado!\n");
        }
        else if(strncmp(comando,"touch ",6)==0){
            char *nome = comando+6;
            int r = criar_arquivo("meu_hd_virtual.bin", nome, diretorio_atual_id);
            if(r!=-1)
                printf("Arquivo criado!\n");
        }
        else if(strcmp(comando,"ls")==0){
            listar_diretorio("meu_hd_virtual.bin", diretorio_atual_id);
        }
        else if(strncmp(comando,"cd ",3)==0){
            mudar_diretorio("meu_hd_virtual.bin", comando+3, &diretorio_atual_id);
        }
        else if(strcmp(comando,"pwd")==0){
            obter_caminho_atual("meu_hd_virtual.bin", diretorio_atual_id, caminho_atual);
            printf("%s\n", caminho_atual);
        }
        else if(strncmp(comando,"importar ",9)==0){
            char real[100];
            char virtual[100];
            if(sscanf(comando+9, "%s %s", real, virtual)==2){
                importar_arquivo_real("meu_hd_virtual.bin", real, virtual, diretorio_atual_id);
                printf("Arquivo importado!\n");
            }
        }
        else if(strncmp(comando,"cat ",4)==0){
            int id = buscar_filho_por_nome("meu_hd_virtual.bin", diretorio_atual_id, comando+4);
            if(id!=-1)
                exibir_conteudo_arquivo("meu_hd_virtual.bin", id);
            else
                printf("Arquivo nao encontrado.\n");
        }
        else if(strncmp(comando,"rm ",3)==0){
            remover_arquivo("meu_hd_virtual.bin", comando+3, diretorio_atual_id);
            printf("Arquivo removido.\n");
        }
        else if(strncmp(comando,"rmdir ",6)==0){
            remover_diretorio("meu_hd_virtual.bin", comando+6, diretorio_atual_id);
            printf("Diretorio removido.\n");
        }
        else if(strncmp(comando,"renomear ",9)==0){
            char antigo[100];
            char novo[100];
            sscanf(comando+9, "%s %s", antigo, novo);
            renomear_item("meu_hd_virtual.bin", antigo, novo, diretorio_atual_id);
            printf("Renomeado!\n");
        }
        else if(strcmp(comando,"sair")==0){
            printf("Encerrando simulador...\n");
            break;
        }
        else{
            printf("Comando nao reconhecido. Digite 'ajuda'.\n");
        }
    }

    if(stream_entrada != stdin)
        fclose(stream_entrada);
    return 0;
}