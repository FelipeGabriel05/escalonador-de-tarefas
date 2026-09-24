#ifndef LEITURA_H
#define LEITURA_H
#include <stdio.h>
#include <stdlib.h>
#include "../modelo/processo.h"
#include "../modelo/configuracao.h"

Processo *ler_processos(
    const char *nome_arquivo, 
    int *quantidade
);

Configuracao ler_configuracao(
    const char *nome_arquivo
);
#endif