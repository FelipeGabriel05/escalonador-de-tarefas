# Nome do executável
TARGET = programa

# Compilador
CC = gcc

# Flags
CFLAGS = -Wall -Wextra -std=c11

# Diretórios dos headers
CFLAGS += -I.
CFLAGS += -Ileitura
CFLAGS += -Imodelo
CFLAGS += -Iescalonadores

# Arquivos fonte
SRCS = main.c \
       $(wildcard leitura/*.c) \
       $(wildcard escalonadores/*.c)

# Arquivos objeto
OBJS = $(SRCS:.c=.o)

# Regra principal
all: $(TARGET)

# Linkagem
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compilação
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza
clean:
	del /Q $(OBJS) $(TARGET).exe 2>nul