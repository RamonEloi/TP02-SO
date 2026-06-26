#ifndef ARQUIVO_H
#define ARQUIVO_H

#include "common.h"
#include "i-node.h"
#include "disco.h"

int criar_arquivo(const char *nome_disco, const char *nome_arquivo, int id_pai);
void exibir_conteudo_arquivo(const char *nome_disco, int id_inode);
int importar_arquivo_real(const char *nome_disco, const char *caminho_real, const char *nome_virtual, int id_pai);

#endif