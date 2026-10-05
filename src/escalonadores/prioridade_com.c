#include "prioridade_com.h"
#include <stdlib.h>

// desempate entre processos de mesma prioridade
// 1) quem ja ta na cpu, 2) menor tempo restante, 3) aleatorio
static int desempate(Processo *p, int escolhido, int candidato, int atual) {
    if (atual == escolhido) return escolhido; //regra 1 (quem já tem a cpu continua)
    if (atual == candidato) return candidato; //ainda regra 1, o atual pode ser o escolhido ou o candidato

    if (p[candidato].restante < p[escolhido].restante) //regra 2 (quem ta mais perto de terminar continua)
        return candidato;

    if (p[candidato].restante == p[escolhido].restante) { // escolha aleatoria
        if (rand() % 2 == 0) return candidato;
    }
    return escolhido;
}

// maior prioridade entre os que ja chegaram
// como e preemptivo, isso e chamado a cada segundo
static int escolher_processo(Processo *p, int n, int tempo, int atual) {
    int escolhido = -1; //se nao tiver nenhum processo

    for (int i = 0; i < n; i++) {
        if (p[i].restante <= 0) continue; //excecao (se o restante for <= 0 significa que o o processo ja finalizou)
        if (p[i].criacao > tempo) continue; //excecao (se a criacao > tempo significa que nao chegou)
	
	//primeiro valido, logo, eh o escolhido, por enquanto
        if (escolhido == -1) {
            escolhido = i;
            continue;
        }

        if (p[i].prioridade > p[escolhido].prioridade) //tem um com maior prioridade,escolhido
            escolhido = i;
        else if (p[i].prioridade == p[escolhido].prioridade) //os dois tem a mesma prioridade, desempate
            escolhido = desempate(p, escolhido, i, atual);
    }
    return escolhido;
}

Resultado executar_prioridade_com(Processo *processos, int quantidade) {
    Resultado resultado;
    resultado.tempo_medio_turnaround = 0; //inicia com 0 
    resultado.tempo_medio_espera = 0;
    resultado.tempo_medio_resposta = 0;
    resultado.trocas_contexto = 0;
    resultado.execucao = NULL;
    resultado.tamanho_execucao = 0;

    if (quantidade <= 0) return resultado; //se nao tiver nenhum processo, devolve zerado

    Processo *copia = malloc(quantidade * sizeof(Processo)); //cria um malloc com a copia para nao alterar os dados originais
    if (copia == NULL) return resultado;

    for (int i = 0; i < quantidade; i++)
        copia[i] = processos[i]; //processo por processo

//quanto tempo total vai durar a linha do tempo (soma todos os valores da criacao)
    int tempo_total = 0;
    for (int i = 0; i < quantidade; i++) {
        tempo_total += copia[i].duracao;
        if (copia[i].criacao > tempo_total)
            tempo_total = copia[i].criacao;
    }
    tempo_total += 1;

	//reserva a memoria 
    resultado.execucao = malloc(tempo_total * sizeof(int));
    if (resultado.execucao == NULL) {
        free(copia);
        return resultado;
    }

    int tempo = 0; // relogio da simulacao, começa em 0
    int finalizados = 0; //quantos processos ja terminaram
    int atual = -1; //-1 se ninguem
    int ultimo = -1; //ultimo processo que executou (para contar trocas apos termino)

    while (finalizados < quantidade) {
        // reavalia toda hora, se chegar alguem mais prioritario ele assume
        int novo = escolher_processo(copia, quantidade, tempo, atual);

        if (novo == -1) {   // cpu ociosa
            resultado.execucao[resultado.tamanho_execucao] = -1; //coloca -1 na proxima casa vazia
            resultado.tamanho_execucao++; //avanca para a casa seguinte
            tempo++; //incrementa para nao ficar preso no mesmo minuto pra sempre
            atual = -1; //não tem ninguém na CPU agora
            // NAO zera 'ultimo': uma troca P1 -> ocioso -> P2 ainda conta,
            // igual aos demais algoritmos
            continue;
        }

        // se o processo na CPU mudou (por preempcao OU porque o anterior terminou), houve troca
        if (ultimo != -1 && ultimo != novo)
            resultado.trocas_contexto++;

        atual = novo;
        ultimo = novo; //guarda o ultimo que executou

	//se eh a primeira vez, registra o instante
        if (copia[atual].inicio == -1)
            copia[atual].inicio = tempo;

        resultado.execucao[resultado.tamanho_execucao] = copia[atual].id; //neste minuto quem rodou foi o processo de id tal
        resultado.tamanho_execucao++;  					  //move-se para a proxima casa
        copia[atual].restante--; //ele usou 1 unidade de CPU ent diminui o restante
        tempo++; //tempo incrementa pq passou 1 unidade de tempo

        if (copia[atual].restante == 0) { //se ele acabou:
            copia[atual].fim = tempo; //registra o fim
            finalizados++; //aumenta a quantidade de finalizados
            atual = -1; //diz que a cpu voltou a ficar livre para o proximo
        }
    }

	//calculo do turnaround, espere, resposta
    int soma_turnaround = 0, soma_espera = 0, soma_resposta = 0;
    for (int i = 0; i < quantidade; i++) {
        int turnaround = copia[i].fim - copia[i].criacao;
        int espera = turnaround - copia[i].duracao;
        int resposta = copia[i].inicio - copia[i].criacao;
        soma_turnaround += turnaround;
        soma_espera += espera;
        soma_resposta += resposta;
    }
	//media do turnaround,espera e resposta
    resultado.tempo_medio_turnaround = (double)soma_turnaround / quantidade;
    resultado.tempo_medio_espera = (double)soma_espera / quantidade;
    resultado.tempo_medio_resposta = (double)soma_resposta / quantidade;

    free(copia); //pra liberar o malloc
    return resultado;
}