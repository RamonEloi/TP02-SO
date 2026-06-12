# Variáveis do Compilador e Flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Nome do executável final
TARGET = simulador

# Busca automaticamente TODOS os arquivos .c na pasta atual
SRCS = $(wildcard *.c)

# Converte a lista de .c para .o (arquivos objeto)
OBJS = $(SRCS:.c=.o)

# Regra padrão: compila o projeto completo
all: $(TARGET)

# Regra para construir o executável final ligando os objetos
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Regra genérica para compilar qualquer arquivo .c em um .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para limpar os arquivos gerados pela compilação (limpeza de ambiente)
clean:
	rm -f $(OBJS) $(TARGET)

# Regra para compilar e rodar o simulador logo em seguida
run: $(TARGET)
	./$(TARGET)

# Declaração de regras que não representam arquivos físicos
.PHONY: all clean run