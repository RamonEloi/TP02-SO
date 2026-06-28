#ifndef MENU_H
#define MENU_H

#include <stdio.h>

void mostrar_menu(int tamanho_disco, int tamanho_bloco, int max_inodes);
void iniciar_menu_interativo(FILE *stream_entrada, int tamanho_disco, int tamanho_bloco, int max_inodes);

#endif