#include "srtf.h"

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
        Primeiro critério: Se um dos processos empatados já está utilizando
        a CPU, ele obrigatoriamente continua executando.
        Isso evita uma troca de contexto desnecessária.
    */
    if (processo_atual == escolhido) {
        return escolhido;
    }

    if (processo_atual == candidato) {
        return candidato;
    }


    /*
        Segundo critério: escolhemos o processo com menor tempo restante.
    */
    if (processos[candidato].restante <
        processos[escolhido].restante) {

        return candidato;
    }


    /*
        Terceiro critério: Se os tempos restantes também forem iguais,
        fazemos uma escolha aleatória.
    */
    if (processos[candidato].restante ==
        processos[escolhido].restante) {

        if (rand() % 2 == 0) {
            return candidato;
        }
    }


    return escolhido;
}


/*
    Escolhe o processo que deverá utilizar a CPU.
    No SRTF, o processo com menor tempo restante
    possui preferência.

    Como o algoritmo é preemptivo, essa função é
    chamada a cada segundo, mesmo quando já existe
    um processo utilizando a CPU.

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


    // Percorremos todos os processos.
    for (int i = 0; i < quantidade; i++) {

        // Processo já terminou.
        if (processos[i].restante <= 0) {
            continue;
        }


        // Processo ainda não chegou.
        if (processos[i].criacao > tempo) {
            continue;
        }


        // Primeiro processo disponível.
        if (escolhido == -1) {
            escolhido = i;
            continue;
        }


        // Critério de menor tempo restante.
        if (processos[i].restante <
            processos[escolhido].restante) {

            escolhido = i;
        }


        // Se os tempos restantes forem iguais, utilizamos os critérios de desempate.
        else if (
            processos[i].restante ==
            processos[escolhido].restante
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
    Executa o algoritmo SRTF
    (Shortest Remaining Time First).

    O SRTF é preemptivo.

    A cada segundo verificamos qual processo disponível
    possui o menor tempo restante.

    Um processo pode ser interrompido caso outro processo
    possua um tempo restante menor.

    Em caso de empate, o processo que já estiver utilizando
    a CPU deve continuar executando.
*/
Resultado executar_srtf(Processo *processos, int quantidade) {

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
        Assim podemos alterar restante, inicio e fim sem modificar os processos originais.
    */
    Processo *copia = malloc(quantidade * sizeof(Processo));

    if (copia == NULL) {
        return resultado;
    }


    // Copiamos os processos.
    for (int i = 0; i < quantidade; i++) {
        copia[i] = processos[i];
    }


    // Calculamos uma capacidade inicial para o diagrama.
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


    //Quantidade de processos finalizados.
    int finalizados = 0;


    // Processo atualmente utilizando a CPU. -1 significa que a CPU está livre.
    int processo_atual = -1;


    /*
        Guarda o último processo que realmente utilizou
        a CPU. É usado para contabilizar trocas de contexto.
    */
    int ultimo_processo = -1;

    // A simulação continua até todos os processos terminarem.
    while (finalizados < quantidade) {

        // O SRTF reavalia a escolha a cada segundo.
        int novo_processo = escolher_processo(
            copia,
            quantidade,
            tempo,
            processo_atual
        );


        // Nenhum processo está disponível. A CPU fica ociosa.
        if (novo_processo == -1) {

            resultado.execucao[
                resultado.tamanho_execucao
            ] = -1;

            resultado.tamanho_execucao++;

            tempo++;

            /*
                CPU ociosa: ninguem esta executando agora.
                NAO zeramos ultimo_processo: se depois da
                ociosidade um processo diferente assumir, isso
                ainda e uma troca (mesma regra dos demais algoritmos).
            */
            processo_atual = -1;

            continue;
        }


        /*
            Verificamos se ocorreu uma troca de contexto.

            Existe troca sempre que o processo na CPU muda em
            relação ao ULTIMO que executou, seja por preempção
            ou porque o anterior terminou e outro assumiu.

            Se a CPU estava ociosa (ultimo_processo == -1),
            não contamos como troca.
        */
        if (
            ultimo_processo != -1 &&
            ultimo_processo != novo_processo
        ) {
            resultado.trocas_contexto++;
        }

        // Atualizamos o processo atual e o ultimo executado.
        processo_atual = novo_processo;
        ultimo_processo = novo_processo;

        // Se é a primeira vez que esse processo recebe a CPU, registramos seu instante de início.
        if (copia[processo_atual].inicio == -1) {
            copia[processo_atual].inicio = tempo;
        }


        // Executamos o processo durante um segundo.
        resultado.execucao[resultado.tamanho_execucao] = copia[processo_atual].id;
        resultado.tamanho_execucao++;


        // Consumimos um segundo do tempo restante.
        copia[processo_atual].restante--;


        tempo++;


        // Verificamos se o processo terminou.
        if (copia[processo_atual].restante == 0) {

            // Registramos o instante de término.
            copia[processo_atual].fim = tempo;


            // Aumentamos a quantidade de processos finalizados.
            finalizados++;


            // Guardamos o processo que acabou de terminar.
            ultimo_processo = processo_atual;


            // A CPU fica livre. Na próxima iteração o SRTF escolherá outro processo.
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


        // Tempo de resposta: primeiro instante em que recebeu CPU - instante de criação.
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