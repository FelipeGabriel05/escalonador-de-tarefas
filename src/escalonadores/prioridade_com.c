#include "prioridade_com.h"
#include <stdlib.h>


/*
    Resolve o empate entre dois processos de mesma prioridade.
    A ordem dos criterios eh a definida pelo trabalho:

    1. processo que ja esteja utilizando a CPU;
    2. menor tempo restante de processamento;
    3. escolha aleatoria.
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
    Escolhe o processo de MAIOR prioridade entre os que ja
    chegaram e ainda nao terminaram.

    Como o algoritmo eh preemptivo, esta funcao eh chamada a cada
    segundo, mesmo que ja exista um processo na CPU. Assim, se
    chega um processo mais prioritario, ele assume o processador
    no mesmo instante.
*/
static int escolher_processo(
    Processo *processos,
    int quantidade,
    int tempo,
    int processo_atual
) {

    int escolhido = -1;

    for (int i = 0; i < quantidade; i++) {

        if (processos[i].restante <= 0) {
            continue;
        }

        if (processos[i].criacao > tempo) {
            continue;
        }

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
            Mesma prioridade: criterios de desempate.
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
    Executa o escalonamento por prioridade COM preempcao.

    A cada segundo verificamos qual processo disponivel tem a
    maior prioridade. Se for diferente do que esta na CPU, ocorre
    preempcao: o processo atual eh interrompido e volta para a
    fila, e o mais prioritario passa a executar.
*/
Resultado executar_prioridade_com(Processo *processos, int quantidade) {

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

    Processo *copia = malloc(quantidade * sizeof(Processo));

    if (copia == NULL) {
        return resultado;
    }

    for (int i = 0; i < quantidade; i++) {
        copia[i] = processos[i];
    }

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

    while (finalizados < quantidade) {

        /*
            Reavaliamos a escolha a cada segundo (preempcao).
        */
        int novo_processo = escolher_processo(
            copia,
            quantidade,
            tempo,
            processo_atual
        );

        /*
            Nenhum processo disponivel: CPU ociosa por 1s.
        */
        if (novo_processo == -1) {

            resultado.execucao[resultado.tamanho_execucao] = -1;
            resultado.tamanho_execucao++;
            tempo++;

            /*
                CPU ficou livre; a proxima entrada nao deve ser
                contada como troca de contexto.
            */
            processo_atual = -1;
            continue;
        }

        /*
            Houve troca de contexto se ja havia um processo na CPU
            e agora outro assume o lugar dele. Quando a CPU estava
            livre (processo terminou ou ficou ociosa), a proxima
            entrada nao eh contada como troca. Mesmo criterio usado
            nos demais escalonadores da equipe.
        */
        if (
            processo_atual != -1 &&
            processo_atual != novo_processo
        ) {
            resultado.trocas_contexto++;
        }

        processo_atual = novo_processo;

        /*
            Primeira vez que este processo pega a CPU: registra o
            instante de inicio (usado no tempo de resposta).
        */
        if (copia[processo_atual].inicio == -1) {
            copia[processo_atual].inicio = tempo;
        }

        /*
            Executa por 1 segundo.
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
