#include "sjf.h"
#include <stdlib.h>

/*
    Resolve o empate entre dois processos.
    A ordem dos critérios definida pelo trabalho é:
    1. processo que já esteja utilizando a CPU;
    2. menor tempo restante de processamento;
    3. escolha aleatória.
*/
static int desempate(
    Processo *processos,
    int escolhido,
    int candidato,
    int processo_atual
) {

    /*
        Primeiro critério: Se o processo que já está utilizando a CPU
        estiver entre os candidatos, ele permanece.
    */
    if (processo_atual == escolhido) {
        return escolhido;
    }

    if (processo_atual == candidato) {
        return candidato;
    }


    // Segundo critério: Escolhemos o processo com menor tempo restante.
    if (processos[candidato].restante <
        processos[escolhido].restante) {

        return candidato;
    }

    if (processos[escolhido].restante <
        processos[candidato].restante) {

        return escolhido;
    }


    /*
        Terceiro critério: Se os tempos restantes também forem iguais,
        escolhemos aleatoriamente entre os dois.
    */
    if (rand() % 2 == 0) {
        return candidato;
    }

    return escolhido;
}


/*
    Escolhe o processo que deverá receber a CPU.
    No SJF, entre os processos que já chegaram,
    escolhemos aquele que possui a menor duração.

    Caso exista empate na duração, utilizamos os
    critérios de desempate definidos no trabalho.

    Retorna o índice do processo escolhido.
    Retorna -1 caso nenhum processo esteja disponível.
*/
static int escolher_processo(
    Processo *processos,
    int quantidade,
    int tempo,
    int processo_atual
) {

    int escolhido = -1;

    for (int i = 0; i < quantidade; i++) {

        // Processos que já terminaram não podem ser escolhidos novamente.
        if (processos[i].restante <= 0) {
            continue;
        }


        // Processos que ainda não chegaram não estão disponíveis.
        if (processos[i].criacao > tempo) {
            continue;
        }


        // Se ainda não temos um processo escolhido, o processo atual passa a ser o candidato.
        if (escolhido == -1) {
            escolhido = i;
            continue;
        }


        /*
            Critério do SJF: menor duração.
        */
        if (processos[i].duracao <
            processos[escolhido].duracao) {

            escolhido = i;
        }


        // Se os dois processos possuem a mesma duração, aplicamos a regra de desempate.
        else if (
            processos[i].duracao ==
            processos[escolhido].duracao
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
    Executa o algoritmo SJF (Shortest Job First).

    O SJF é um algoritmo não preemptivo.

    Isso significa que, depois que um processo começa
    a executar, ele permanece utilizando a CPU até terminar.

    Recebe:
        processos  -> vetor com os processos
        quantidade -> quantidade de processos

    Retorna:
        Resultado contendo:
        - tempo médio de turnaround
        - tempo médio de espera
        - tempo médio de resposta
        - número de trocas de contexto
        - diagrama da execução
*/
Resultado executar_sjf(Processo *processos, int quantidade) {

    Resultado resultado;

    resultado.tempo_medio_turnaround = 0;
    resultado.tempo_medio_espera = 0;
    resultado.tempo_medio_resposta = 0;

    resultado.trocas_contexto = 0;

    resultado.execucao = NULL;
    resultado.tamanho_execucao = 0;


    // Não há nada para executar se não existem processos.
    if (quantidade <= 0) {
        return resultado;
    }


    /*
        Criamos uma cópia dos processos.

        Durante a simulação precisamos alterar:
        - tempo restante;
        - instante de início;
        - instante de término.

        A cópia evita modificar os processos originais.
    */
    Processo *copia = malloc(quantidade * sizeof(Processo));

    if (copia == NULL) {
        return resultado;
    }

    // Copiamos os processos.
    for (int i = 0; i < quantidade; i++) {
        copia[i] = processos[i];
    }


    /*
        Calculamos uma capacidade inicial para o diagrama.
        Consideramos também possíveis períodos em que
        a CPU ficará ociosa antes da chegada dos processos.
    */
    int tempo_total = 0;

    for (int i = 0; i < quantidade; i++) {

        tempo_total += copia[i].duracao;

        if (copia[i].criacao > tempo_total) {
            tempo_total = copia[i].criacao;
        }
    }

    tempo_total += 1;


    // Alocamos memória para o diagrama.
    resultado.execucao = malloc(tempo_total * sizeof(int));

    if (resultado.execucao == NULL) {
        free(copia);
        return resultado;
    }


    int tempo = 0;


    // Quantidade de processos que já terminaram.
    int finalizados = 0;


    // Processo que está atualmente utilizando a CPU. -1 significa que a CPU está livre.
    int processo_atual = -1;


    /*
        Processo que estava utilizando a CPU anteriormente.
        É utilizado para contabilizar trocas de contexto.
    */
    int processo_anterior = -1;


    // A simulação continua até que todos os processos tenham terminado.
    while (finalizados < quantidade) {


        // Como o SJF é não preemptivo, somente escolhemos um novo processo quando a CPU está livre.
        if (processo_atual == -1) {

            processo_atual = escolher_processo(
                copia,
                quantidade,
                tempo,
                processo_atual
            );


            /*
                Nenhum processo está disponível.
                Nesse caso a CPU fica ociosa durante
                um segundo.
            */
            if (processo_atual == -1) {
                resultado.execucao[resultado.tamanho_execucao] = -1;
                resultado.tamanho_execucao++;
                tempo++;
                continue;
            }


            /*
                Se é a primeira vez que o processo recebe
                a CPU, registramos seu instante de início.
                Esse valor será utilizado para calcular
                o tempo de resposta.
            */
            if (copia[processo_atual].inicio == -1) {
                copia[processo_atual].inicio = tempo;
            }


            /*
                Se já existia um processo utilizando a CPU
                anteriormente e agora outro processo assumiu,
                ocorreu uma troca de contexto.
            */
            if (
                processo_anterior != -1 &&
                processo_anterior != processo_atual
            ) {
                resultado.trocas_contexto++;
            }
        }


        // O processo atual executa durante um segundo. Guardamos seu ID no diagrama.
        resultado.execucao[resultado.tamanho_execucao] = copia[processo_atual].id;
        resultado.tamanho_execucao++;


        // Um segundo de processamento foi consumido.
        copia[processo_atual].restante--;

        tempo++;


        // Verificamos se o processo terminou.
        if (copia[processo_atual].restante == 0) {

            // O processo terminou no instante atual.
            copia[processo_atual].fim = tempo;


            // Atualizamos a quantidade de processos finalizados.
            finalizados++;


            // Guardamos qual processo estava utilizando a CPU antes dela ficar livre.
            processo_anterior = processo_atual;


            // Como o processo terminou, a CPU ficará livre. Na próxima iteração será escolhido outro processo.
            processo_atual = -1;
        }
    }


    int soma_turnaround = 0;
    int soma_espera = 0;
    int soma_resposta = 0;


    for (int i = 0; i < quantidade; i++) {

        // Turnaround: tempo de término - tempo de criação.
        int turnaround = copia[i].fim - copia[i].criacao;


        // Tempo de espera: turnaround - duração.
        int espera = turnaround - copia[i].duracao;


        //Tempo de resposta: primeiro instante em que recebeu CPU - instante de criação.
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