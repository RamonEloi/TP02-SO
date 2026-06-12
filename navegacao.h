#ifndef NAVEGACAO_H
#define NAVEGACAO_H

#include "common.h"
#include "i-node.h"
#include "disco.h"
#include "diretorio.h"

int encontrar_inode_por_caminho(const char* nome_disco, const char* caminho, int id_diretorio_atual);
int simular_mkdir(const char* nome_disco, const char* nome_novo_dir, int id_pai);
void listar_diretorio(const char* nome_disco, int id_diretorio);

#endif