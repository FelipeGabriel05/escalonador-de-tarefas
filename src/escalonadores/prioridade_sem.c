#include "prioridade_sem.h"
#include <stdlib.h>


/*
    Resolve o empate entre dois processos que possuem a mesma
    prioridade. A ordem dos criterios eh a definida pelo trabalho:

    1. processo que ja esteja utilizando a CPU;
    2. menor tempo restante de processamento;
    3. escolha aleatoria.

    No algoritmo sem preempcao a decisao soh acontece quando a CPU
    esta livre, entao o criterio 1 raramente se aplica, mas mantemos
    a mesma funcao de desempate usada nos outros escalonadores.
*/
static int desempate(
    Processo *processos,
    int escolhido,
    int candidato,
    int processo_atual
) {

    if (processo_atual == escolhido) {
        return escolhido;
    }

    if (processo_atual == candidato) {
        return candidato;
    }

    if (processos[candidato].restante <
        processos[escolhido].restante) {

        return candidato;
    }

    if (processos[candidato].restante ==
        processos[escolhido].restante) {

        if (rand() % 2 == 0) {
            return candidato;
        }
    }

    return escolhido;
}


/*
    Escolhe, entre os processos que ja chegaram e ainda nao
    terminaram, aquele de MAIOR prioridade.

    Seguimos a convencao dos slides da disciplina: quanto maior
    o valor da prioridade, mais importante eh o processo.

    Retorna o indice do processo escolhido, ou -1 se nenhum
    processo estiver disponivel neste instante.
*/
static int escolher_processo(
    Processo *processos,
    int quantidade,
    int tempo,
    int processo_atual
) {

    int escolhido = -1;

    for (int i = 0; i < quantidade; i++) {

        /*
            Processo ja terminou.
        */
        if (processos[i].restante <= 0) {
            continue;
        }

        /*
            Processo ainda nao chegou.
        */
        if (processos[i].criacao > tempo) {
            continue;
        }

        /*
            Primeiro processo disponivel.
        */
        if (escolhido == -1) {
            escolhido = i;
            continue;
        }

        /*
            Criterio principal: maior prioridade.
        */
        if (processos[i].prioridade >
            processos[escolhido].prioridade) {

            escolhido = i;
        }

        /*
            Mesma prioridade: aplicamos os criterios de desempate.
        */
        else if (
            processos[i].prioridade ==
            processos[escolhido].prioridade
        ) {

            escolhido = desempate(
                processos,
                escolhido,
                i,
                processo_atual
            );
        }
    }

    return escolhido;
}


/*
    Executa o escalonamento por prioridade SEM preempcao.

    Uma vez que um processo recebe a CPU, ele executa ate
    terminar. Soh quando ele termina eh que escolhemos o
    proximo (o de maior prioridade entre os que ja chegaram).
*/
Resultado executar_prioridade_sem(Processo *processos, int quantidade) {

    Resultado resultado;

    resultado.tempo_medio_turnaround = 0;
    resultado.tempo_medio_espera = 0;
    resultado.tempo_medio_resposta = 0;
    resultado.trocas_contexto = 0;
    resultado.execucao = NULL;
    resultado.tamanho_execucao = 0;

    if (quantidade <= 0) {
        return resultado;
    }

    /*
        Copia dos processos: durante a simulacao alteramos
        restante, inicio e fim, e nao queremos estragar o vetor
        original, que ainda sera usado por outros algoritmos.
    */
    Processo *copia = malloc(quantidade * sizeof(Processo));

    if (copia == NULL) {
        return resultado;
    }

    for (int i = 0; i < quantidade; i++) {
        copia[i] = processos[i];
    }

    /*
        Estimativa do tamanho do diagrama: soma das duracoes,
        respeitando um possivel tempo ocioso inicial.
    */
    int tempo_total = 0;
    for (int i = 0; i < quantidade; i++) {
        tempo_total += copia[i].duracao;

        if (copia[i].criacao > tempo_total) {
            tempo_total = copia[i].criacao;
        }
    }
    tempo_total += 1;

    resultado.execucao = malloc(tempo_total * sizeof(int));

    if (resultado.execucao == NULL) {
        free(copia);
        return resultado;
    }

    int tempo = 0;
    int finalizados = 0;
    int processo_atual = -1;
    int processo_anterior = -1;

    while (finalizados < quantidade) {

        /*
            Como nao ha preempcao, so escolhemos um novo processo
            quando a CPU esta livre.
        */
        if (processo_atual == -1) {

            processo_atual = escolher_processo(
                copia,
                quantidade,
                tempo,
                processo_atual
            );

            /*
                Nenhum processo chegou ainda: CPU ociosa por 1s.
            */
            if (processo_atual == -1) {

                resultado.execucao[resultado.tamanho_execucao] = -1;
                resultado.tamanho_execucao++;
                tempo++;
                continue;
            }

            /*
                Primeira vez que este processo pega a CPU:
                registramos o instante de inicio (para o tempo
                de resposta).
            */
            if (copia[processo_atual].inicio == -1) {
                copia[processo_atual].inicio = tempo;
            }

            /*
                Troca de contexto: a CPU mudou de dono. A primeira
                carga (anterior == -1) nao conta como troca.
            */
            if (
                processo_anterior != -1 &&
                processo_anterior != processo_atual
            ) {
                resultado.trocas_contexto++;
            }
        }

        /*
            Executa o processo atual por 1 segundo.
        */
        resultado.execucao[resultado.tamanho_execucao] = copia[processo_atual].id;
        resultado.tamanho_execucao++;

        copia[processo_atual].restante--;
        tempo++;

        /*
            Terminou: registra o fim e libera a CPU.
        */
        if (copia[processo_atual].restante == 0) {

            copia[processo_atual].fim = tempo;
            finalizados++;
            processo_anterior = processo_atual;
            processo_atual = -1;
        }
    }

    int soma_turnaround = 0;
    int soma_espera = 0;
    int soma_resposta = 0;

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
