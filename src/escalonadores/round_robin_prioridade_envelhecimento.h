#ifndef ROUND_ROBIN_PRIORIDADE_ENVELHECIMENTO_H
#define ROUND_ROBIN_PRIORIDADE_ENVELHECIMENTO_H

#include "../modelo/processo.h"
#include "../modelo/configuracao.h"
#include "../modelo/resultado.h"

// Round-Robin com prioridade e envelhecimento (aging)
// O quantum e a taxa de envelhecimento vêm da configuração
// Não há preempção por prioridade
Resultado executar_round_robin_prioridade_envelhecimento(
    Processo *processos,
    int quantidade,
    Configuracao config
);

#endif
