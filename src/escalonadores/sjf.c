#include "sjf.h"

#include <stdlib.h>


/*
 * Escolhe o processo que deverá receber a CPU.
 *
 * No SJF, entre os processos que já chegaram,
 * escolhemos aquele que possui a menor duração.
 *
 * Caso dois processos tenham a mesma duração,
 * usamos o menor ID como desempate.
 *
 * Retorna o índice do processo escolhido.
 *
 * Retorna -1 caso nenhum processo esteja disponível.
 */
static int escolher_processo(Processo *processos, int quantidade, int tempo) {

    int escolhido = -1;

    for (int i = 0; i < quantidade; i++) {

        /*
         * Processo já terminou.
         */
        if (processos[i].restante <= 0) {
            continue;
        }

        /*
         * Processo ainda não chegou.
         */
        if (processos[i].criacao > tempo) {
            continue;
        }

        /*
         * Primeiro processo disponível encontrado.
         */
        if (escolhido == -1) {
            escolhido = i;
            continue;
        }

        /*
         * Critério principal do SJF:
         * menor duração.
         */
        if (processos[i].duracao < processos[escolhido].duracao) {
            escolhido = i;
        }

        /*
         * Desempate:
         * menor ID.
         */
        else if (processos[i].duracao == processos[escolhido].duracao &&
                 processos[i].id < processos[escolhido].id) {

            escolhido = i;
        }
    }

    return escolhido;
}


Resultado executar_sjf(Processo *processos, int quantidade) {

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
     * Criamos uma cópia para podermos alterar
     * restante, início e fim sem modificar
     * os processos originais.
     */
    Processo *copia = malloc(quantidade * sizeof(Processo));

    if (copia == NULL) {
        return resultado;
    }


    for (int i = 0; i < quantidade; i++) {
        copia[i] = processos[i];
    }


    /*
     * Por enquanto usamos a soma das durações
     * como capacidade do diagrama.
     */
    int tempo_total = 0;

    for (int i = 0; i < quantidade; i++) {
        tempo_total += copia[i].duracao;
    }


    resultado.execucao = malloc(tempo_total * sizeof(int));

    if (resultado.execucao == NULL) {
        free(copia);
        return resultado;
    }


    int tempo = 0;
    int finalizados = 0;

    int processo_atual = -1;
    int processo_anterior = -1;


    /*
     * Continua até todos os processos terminarem.
     */
    while (finalizados < quantidade) {

        /*
         * No SJF não existe preempção.
         *
         * Só escolhemos um novo processo quando
         * a CPU está livre.
         */
        if (processo_atual == -1) {

            processo_atual =
                escolher_processo(copia, quantidade, tempo);


            /*
             * Nenhum processo chegou ainda.
             * CPU fica ociosa.
             */
            if (processo_atual == -1) {

                resultado.execucao[
                    resultado.tamanho_execucao
                ] = -1;

                resultado.tamanho_execucao++;

                tempo++;

                continue;
            }


            /*
             * Primeiro instante em que o processo
             * recebe a CPU.
             */
            if (copia[processo_atual].inicio == -1) {
                copia[processo_atual].inicio = tempo;
            }


            /*
             * Se outro processo estava executando
             * anteriormente, ocorreu troca de contexto.
             */
            if (processo_anterior != -1 &&
                processo_anterior != processo_atual) {

                resultado.trocas_contexto++;
            }
        }


        /*
         * Executa o processo durante 1 segundo.
         */
        resultado.execucao[
            resultado.tamanho_execucao
        ] = copia[processo_atual].id;

        resultado.tamanho_execucao++;


        /*
         * Diminui o tempo restante.
         */
        copia[processo_atual].restante--;


        /*
         * Avança o relógio.
         */
        tempo++;


        /*
         * Processo terminou.
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