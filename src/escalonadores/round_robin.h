#ifndef ROUND_ROBIN_H
#define ROUND_ROBIN_H

#include "../modelo/processo.h"
#include "../modelo/configuracao.h"
#include "../modelo/resultado.h"

// Round-Robin puro: fila FIFO, quantum vindo da configuração, sem prioridade
// (a taxa de aging da configuração é ignorada aqui)
Resultado executar_round_robin(
    Processo *processos,
    int quantidade,
    Configuracao config
);

#endif
