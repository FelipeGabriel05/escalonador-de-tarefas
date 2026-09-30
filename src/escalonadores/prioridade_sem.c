#include "prioridade_sem.h"
#include <stdlib.h>

// desempate quando dois processos tem a mesma prioridade
// 1) quem ja ta na cpu, 2) menor tempo restante, 3) aleatorio
static int desempate(Processo *p, int escolhido, int candidato, int atual) {
    if (atual == escolhido) return escolhido;
    if (atual == candidato) return candidato;

    if (p[candidato].restante < p[escolhido].restante)
        return candidato;

    if (p[candidato].restante == p[escolhido].restante) {
        if (rand() % 2 == 0) return candidato;
    }
    return escolhido;
}

// pega o processo de maior prioridade entre os que ja chegaram
// (maior numero = mais prioritario)
static int escolher_processo(Processo *p, int n, int tempo, int atual) {
    int escolhido = -1;

    for (int i = 0; i < n; i++) {
        if (p[i].restante <= 0) continue;      // ja terminou
        if (p[i].criacao > tempo) continue;    // ainda nao chegou

        if (escolhido == -1) {
            escolhido = i;
            continue;
        }

        if (p[i].prioridade > p[escolhido].prioridade)
            escolhido = i;
        else if (p[i].prioridade == p[escolhido].prioridade)
            escolhido = desempate(p, escolhido, i, atual);
    }
    return escolhido;
}

Resultado executar_prioridade_sem(Processo *processos, int quantidade) {
    Resultado resultado;
    resultado.tempo_medio_turnaround = 0;
    resultado.tempo_medio_espera = 0;
    resultado.tempo_medio_resposta = 0;
    resultado.trocas_contexto = 0;
    resultado.execucao = NULL;
    resultado.tamanho_execucao = 0;

    if (quantidade <= 0) return resultado;

    // trabalha numa copia pra nao estragar o vetor original
    Processo *copia = malloc(quantidade * sizeof(Processo));
    if (copia == NULL) return resultado;

    for (int i = 0; i < quantidade; i++)
        copia[i] = processos[i];

    // tamanho maximo do diagrama
    int tempo_total = 0;
    for (int i = 0; i < quantidade; i++) {
        tempo_total += copia[i].duracao;
        if (copia[i].criacao > tempo_total)
            tempo_total = copia[i].criacao;
    }
    tempo_total += 1;

    resultado.execucao = malloc(tempo_total * sizeof(int));
    if (resultado.execucao == NULL) {
        free(copia);
        return resultado;
    }

    int tempo = 0;
    int finalizados = 0;
    int atual = -1;
    int anterior = -1;

    while (finalizados < quantidade) {
        // sem preempcao: so escolhe outro quando a cpu ta livre
        if (atual == -1) {
            atual = escolher_processo(copia, quantidade, tempo, atual);

            if (atual == -1) {   // ninguem chegou ainda, cpu ociosa
                resultado.execucao[resultado.tamanho_execucao] = -1;
                resultado.tamanho_execucao++;
                tempo++;
                continue;
            }

            if (copia[atual].inicio == -1)
                copia[atual].inicio = tempo;

            // conta troca de contexto (menos a primeira carga)
            if (anterior != -1 && anterior != atual)
                resultado.trocas_contexto++;
        }

        resultado.execucao[resultado.tamanho_execucao] = copia[atual].id;
        resultado.tamanho_execucao++;
        copia[atual].restante--;
        tempo++;

        if (copia[atual].restante == 0) {
            copia[atual].fim = tempo;
            finalizados++;
            anterior = atual;
            atual = -1;
        }
    }

    int soma_turnaround = 0, soma_espera = 0, soma_resposta = 0;
    for (int i = 0; i < quantidade; i++) {
        int turnaround = copia[i].fim - copia[i].criacao;
        int espera = turnaround - copia[i].duracao;
        int resposta = copia[i].inicio - copia[i].criacao;
        soma_turnaround += turnaround;
        soma_espera += espera;
        soma_resposta += resposta;
    }

    resultado.tempo_medio_turnaround = (double)soma_turnaround / quantidade;
    resultado.tempo_medio_espera = (double)soma_espera / quantidade;
    resultado.tempo_medio_resposta = (double)soma_resposta / quantidade;

    free(copia);
    return resultado;
}