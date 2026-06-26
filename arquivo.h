#ifndef ARQUIVO_H
#define ARQUIVO_H

#include "common.h"
#include "i-node.h"
#include "disco.h"

int criar_arquivo(const char *nome_disco, const char *nome_arquivo, int id_pai);
void exibir_conteudo_arquivo(const char *nome_disco, int id_inode);
int importar_arquivo_real(const char *nome_disco, const char *caminho_real, const char *nome_virtual, int id_pai);
int remover_arquivo(const char *nome_disco, const char *nome_arquivo, int id_pai);
int remover_diretorio(const char *nome_disco, const char *nome_pasta, int id_pai);
int renomear_item(const char *nome_disco, const char *nome_antigo, const char *nome_novo, int id_pai);
int mover_item(const char *nome_disco, const char *nome_item, const char *nome_destino, int id_pai_atual);

#endif