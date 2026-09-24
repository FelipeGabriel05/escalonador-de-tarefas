#ifndef PROCESSO_H
#define PROCESSO_H

typedef struct {
    int id;
    int criacao;
    int duracao;
    int prioridade;

    int restante;
    int inicio;
    int fim;
} Processo;

#endif
