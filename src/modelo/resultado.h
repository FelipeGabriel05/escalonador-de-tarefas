#ifndef RESULTADO_H
#define RESULTADO_H

typedef struct {
    double tempo_medio_turnaround;
    double tempo_medio_espera;
    double tempo_medio_resposta;

    int trocas_contexto;

    int *execucao;
    int tamanho_execucao;
} Resultado;

#endif