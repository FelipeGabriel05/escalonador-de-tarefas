#ifndef FCFS_H
#define FCFS_H
#include <stdlib.h>
#include "../modelo/processo.h"
#include "../modelo/resultado.h"

Resultado executar_fcfs(Processo *processos, int quantidade);
#endif