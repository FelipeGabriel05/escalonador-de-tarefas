#ifndef ROUND_ROBIN_H
#define ROUND_ROBIN_H

#include "../modelo/processo.h"
#include "../modelo/configuracao.h"
#include "../modelo/resultado.h"

// Round-Robin puro: fila FIFO, quantum vindo da configuração, sem prioridade
Resultado executar_round_robin(
    Processo *processos,
    int quantidade,
    Configuracao config
);

// Round-Robin com prioridade e envelhecimento (aging)
// O quantum e a taxa de envelhecimento vêm da configuração
// Não há preempção por prioridade
Resultado executar_round_robin_prioridade(
    Processo *processos,
    int quantidade,
    Configuracao config
);

#endif
