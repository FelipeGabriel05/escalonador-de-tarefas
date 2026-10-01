#include "round_robin_prioridade_envelhecimento.h"

#include <stdlib.h>
#include <time.h>

/* Cria e inicializa um Resultado vazio para começar a execução
Também é usado caso ocorra algum erro na alocação de memória */ 
static Resultado resultado_vazio(void) {
    Resultado resultado;

    resultado.tempo_medio_turnaround = 0; 
    resultado.tempo_medio_espera = 0;
    resultado.tempo_medio_resposta = 0;

    resultado.trocas_contexto = 0;
    resultado.execucao = NULL;  
    resultado.tamanho_execucao = 0;

    return resultado;
}

/* Cria uma cópia dos processos para que o algoritmo possa alterar
tempo restante, início e fim sem modificar o vetor original */
static Processo *copiar_processos(const Processo *processos, int quantidade) {
    Processo *copia = malloc(quantidade * sizeof(Processo));

    if (copia == NULL) { 
        return NULL; // Se o malloc não conseguiu reservar memória, retorna null
    }

    for (int i = 0; i < quantidade; i++) { 
        copia[i] = processos[i]; // Percorre todos os processos e copia cada um
    }

    return copia;
}

/* Calcula um limite seguro para o tamanho do vetor que registra a sequência
de execução. Pior caso: a CPU fica ociosa até o último processo chegar e
depois executa a soma de todas as durações */
static int calcular_tempo_maximo(const Processo *processos, int quantidade) {
    int ultima_criacao = 0;
    int soma_duracoes = 0;

    for (int i = 0; i < quantidade; i++) { // Percorre todos os processos
        if (processos[i].criacao > ultima_criacao) {
            ultima_criacao = processos[i].criacao; // Procura o processo que chega mais tarde

        }
        if (processos[i].duracao > 0) {
            soma_duracoes += processos[i].duracao; // Soma as durações dos processos
        }
    }

    return ultima_criacao + soma_duracoes; // Retorna a estimativa de tempo máximo
}

// Calcula as métricas de cada processo depois que o escalonamento terminou
static void calcular_metricas(const Processo *copia, int quantidade, Resultado *resultado) {
    int soma_turnaround = 0; 
    int soma_espera = 0;
    int soma_resposta = 0;

    for (int i = 0; i < quantidade; i++) {
        int turnaround = copia[i].fim - copia[i].criacao; // turnaround = fim - chegada
        int espera = turnaround - copia[i].duracao; // espera = turnaround - duração
        int resposta = copia[i].inicio - copia[i].criacao; // resposta = primeira execução - chegada

        // Vai acumulando os valores de todos os processos
        soma_turnaround += turnaround;
        soma_espera += espera;
        soma_resposta += resposta;
    }
    // Depois calcula a média dessas métricas
    resultado->tempo_medio_turnaround = (double)soma_turnaround / quantidade;
    resultado->tempo_medio_espera = (double)soma_espera / quantidade;
    resultado->tempo_medio_resposta = (double)soma_resposta / quantidade;
}

// Processo com duração <= 0 já nasce "pronto", então marca como concluído
static int finalizar_processos_vazios(Processo *copia, int quantidade) {
    int finalizados = 0;

    for (int i = 0; i < quantidade; i++) {
        if (copia[i].restante <= 0) {
            copia[i].inicio = copia[i].criacao;
            copia[i].fim = copia[i].criacao;
            finalizados++;
        }
    }

    return finalizados;
}

// Round-Robin com prioridade e envelhecimento

/*
   Critérios de escolha, em ordem:
   1) maior força (prioridade + aging acumulado)
   2) quem está esperando há mais tempo (pronto_desde menor) -> round-robin dentro da mesma prioridade
   3) menor tempo restante
   4) empate total -> escolha aleatória (regra do enunciado)
*/

static int escolher_processo(
    const Processo *copia,
    int quantidade,
    int tempo,
    const int *forca,
    const int *pronto_desde
) {
    int escolhido = -1; // Ainda não foi escolhido
    int total_empatados = 0; // quantos empataram em tudo com o "escolhido" até agora

    for (int i = 0; i < quantidade; i++) {

        if (copia[i].restante <= 0) { // Ignora processos terminados
            continue; // 
        }

        if (copia[i].criacao > tempo) { // Ignora processos que não chegaram
            continue;
        }

        if (escolhido == -1) {
            escolhido = i;
            total_empatados = 1;
            continue;
        }

        if (forca[i] > forca[escolhido]) { // Se o processo tem força maior, ele é escolhido
            escolhido = i;
            total_empatados = 1;
        } else if (forca[i] == forca[escolhido]) { // Se as forças são iguais
            if (pronto_desde[i] < pronto_desde[escolhido]) { // Quem está esperando há mais tempo ganha
                escolhido = i;
                total_empatados = 1;
            } else if (pronto_desde[i] == pronto_desde[escolhido] && // Se esperam desde o mesmo instante
                       copia[i].restante < copia[escolhido].restante) { // Escolhe o processo com menor tempo restante
                escolhido = i;
                total_empatados = 1;
            } else if (pronto_desde[i] == pronto_desde[escolhido] &&
                       copia[i].restante == copia[escolhido].restante) {
            // Empate total (força, pronto_desde e restante iguais): sorteia entre os empatados
            // Cada novo empatado tem 1/total_empatados de chance de ficar no lugar do escolhido
                total_empatados++;
                if (rand() % total_empatados == 0) {
                    escolhido = i;
                }
            }
        }
    }

    return escolhido;
}

// Executa o Round Robin com prioridade e envelhecimento
Resultado executar_round_robin_prioridade_envelhecimento(Processo *processos, int quantidade, Configuracao config) {
    Resultado resultado = resultado_vazio();

    // Semeia rand() só na primeira chamada.
    static int semente_definida = 0;
    if (!semente_definida) {
        srand((unsigned int)time(NULL));
        semente_definida = 1;
    }

    if (quantidade <= 0 || config.quantum <= 0) { // Se não há processos ou o quantum é inválido, encerra
        return resultado;
    }

    Processo *copia = copiar_processos(processos, quantidade); // Cria a cópia
    int *forca_base = malloc(quantidade * sizeof(int)); // Guarda a prioridade original, que não muda
    int *forca = malloc(quantidade * sizeof(int)); // Guarda a prioridade atual, que pode aumentar pelo aging 
    int *pronto_desde = malloc(quantidade * sizeof(int)); // Guarda desde quando o processo está esperando
    int *execucao = malloc((calcular_tempo_maximo(processos, quantidade) + 1) * sizeof(int)); // Cria vetor de que processo está usando a cpu em cada instante

    // Se algum malloc falhou, libera o que já foi alocado (free em NULL não faz nada) e devolve vazio
    if (copia == NULL || forca_base == NULL || forca == NULL || pronto_desde == NULL || execucao == NULL) {
        free(copia);
        free(forca_base);
        free(forca);
        free(pronto_desde);
        free(execucao);
        return resultado;
    }

    resultado.execucao = execucao;

    // Prepara os três vetores auxiliares, um pra cada processo
    for (int i = 0; i < quantidade; i++) {
        forca_base[i] = copia[i].prioridade; // guarda a prioridade original
        forca[i] = forca_base[i]; // a força começa igual à prioridade, e vai subir com o aging
        pronto_desde[i] = 2 * copia[i].criacao; // marca desde quando ele está esperando (x2 pra desempatar com quem foi interrompido)
    }

    int tempo = 0; // Relógio do sistema
    int ultimo = -1; // Guarda o último processo executado. Começa em -1, pois nenhum processo foi executado ainda
    int finalizados = finalizar_processos_vazios(copia, quantidade); // Conta os processos que já estavam terminados

// Enquanto ainda houver processo não terminado, continue
    while (finalizados < quantidade) {

        int atual = escolher_processo(copia, quantidade, tempo, forca, pronto_desde); // Decide quem vai usar a CPU agora

        if (atual == -1) { // Ninguém pronto: CPU ociosa
            resultado.execucao[resultado.tamanho_execucao] = -1; // Registra cpu ociosa
            resultado.tamanho_execucao++;

            tempo++; // Passa 1 segundo
            continue;
        }

 // Se o processo nunca executou antes, registra o instante da primeira execução
        if (copia[atual].inicio == -1) {
            copia[atual].inicio = tempo;
        }

// Se havia um processo anterior e agora estamos executando outro, contabiliza uma troca de contexto
        if (ultimo != -1 && ultimo != atual) {
            resultado.trocas_contexto++;
        }

// Inicialmente, a fatia é igual ao quantum. O quantum indica quantas unidades de tempo o processo pode executar
        int fatia = config.quantum;
        if (copia[atual].restante < fatia) {
            fatia = copia[atual].restante;
        }

        for (int s = 0; s < fatia; s++) {
            resultado.execucao[resultado.tamanho_execucao] = copia[atual].id;
            resultado.tamanho_execucao++;

            copia[atual].restante--;
            tempo++;
        }

        ultimo = atual; // Guarda quem acabou de executar

        if (copia[atual].restante == 0) { // Depois verifica se o processo terminou
            copia[atual].fim = tempo; // Se terminou, registra o tempo de fim e aumenta o número de processos finalizados
            finalizados++;
        }

        /* Envelhecimento: quem ficou esperando (não é o atual, ainda não
           terminou e já tinha chegado antes desse instante) ganha +aging
           na força */
        for (int i = 0; i < quantidade; i++) {
            if (i != atual && copia[i].restante > 0 && copia[i].criacao < tempo) {
                forca[i] += config.aging; // aumenta a força de quem esperou
            }
        }

        forca[atual] = forca_base[atual]; // quem rodou volta pra força original (não acumula aging à toa)
        pronto_desde[atual] = 2 * tempo + 1; // e vai pro fim da fila da sua classe (round-robin dentro da prioridade)
    }

    calcular_metricas(copia, quantidade, &resultado);

    free(copia);
    free(forca_base);
    free(forca);
    free(pronto_desde);

    return resultado;
}
