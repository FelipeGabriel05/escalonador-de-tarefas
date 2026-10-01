#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "leitura/leitura.h"
#include "escalonadores/fcfs.h"
#include "escalonadores/sjf.h"
#include "escalonadores/srtf.h"
#include "escalonadores/prioridade_sem.h"
#include "escalonadores/prioridade_com.h"
#include "escalonadores/round_robin.h"
#include "escalonadores/round_robin_prioridade_envelhecimento.h"


Processo *carregar_processos(const char *nome_arquivo, int *quantidade) {

    Processo *processos = ler_processos(nome_arquivo, quantidade);
    if (processos == NULL) {
        printf("Erro ao ler processos do arquivo %s.\n", nome_arquivo);
        return NULL;
    }

    printf("\nQuantidade: %d\n\n", *quantidade);

    for (int i = 0; i < *quantidade; i++) {
        printf(
            "P%d: criacao=%d duracao=%d prioridade=%d\n",
            processos[i].id,
            processos[i].criacao,
            processos[i].duracao,
            processos[i].prioridade
        );
    }
    return processos;
}


void mostrar_resultado(
    Resultado resultado,
    Processo *processos,
    int quantidade
) {

    printf("\nTempo medio de turnaround: %.2f\n", resultado.tempo_medio_turnaround);
    printf("Tempo medio de espera: %.2f\n", resultado.tempo_medio_espera);
    printf("Tempo medio de resposta: %.2f\n", resultado.tempo_medio_resposta);
    printf("Trocas de contexto: %d\n", resultado.trocas_contexto);


    /*
        Linha simples pra interface ler o grafico de Gantt.
        Mostra qual processo rodou em cada segundo (-1 = CPU ociosa).
    */
    printf("\nTIMELINE:");
    for (int t = 0; t < resultado.tamanho_execucao; t++) {
        printf("%d ", resultado.execucao[t]);
    }
    printf("\n");


    printf("\nDiagrama:\n\n");

    printf("tempo");
    for (int i = 0; i < quantidade; i++) {
        printf("   P%d", processos[i].id);
    }
    printf("\n");

    for (int tempo = 0; tempo < resultado.tamanho_execucao; tempo++) {

        printf("%d-%-2d", tempo, tempo + 1);

        for (int i = 0; i < quantidade; i++) {

            int id = processos[i].id;
            int ultimo_tempo = -1;

            for (int t = 0; t < resultado.tamanho_execucao; t++) {
                if (resultado.execucao[t] == id) {
                    ultimo_tempo = t;
                }
            }

            int fim = ultimo_tempo + 1;

            if (tempo < processos[i].criacao) {
                printf("    ");
            }
            else if (tempo >= fim) {
                printf("    ");
            }
            else if (resultado.execucao[tempo] == id) {
                printf("   ##");
            }
            else {
                printf("   --");
            }
        }
        printf("\n");
    }

    free(resultado.execucao);
}


/*
    Executa um algoritmo pelo numero da opcao, lendo de um arquivo.
    Usado tanto pelo menu do terminal quanto pela interface.
*/
int executar_opcao(int opcao, const char *arquivo) {

    int quantidade;
    Processo *processos = NULL;
    Configuracao config = ler_configuracao("config.txt");
    Resultado resultado;

    switch (opcao) {
        case 1:
            printf("\n===== FCFS =====\n");
            processos = carregar_processos(arquivo, &quantidade);
            if (processos == NULL) return 1;
            resultado = executar_fcfs(processos, quantidade);
            break;
        case 2:
            printf("\n===== SJF =====\n");
            processos = carregar_processos(arquivo, &quantidade);
            if (processos == NULL) return 1;
            resultado = executar_sjf(processos, quantidade);
            break;
        case 3:
            printf("\n===== SRTF =====\n");
            processos = carregar_processos(arquivo, &quantidade);
            if (processos == NULL) return 1;
            resultado = executar_srtf(processos, quantidade);
            break;
        case 4:
            printf("\n===== Prioridade sem preempcao =====\n");
            processos = carregar_processos(arquivo, &quantidade);
            if (processos == NULL) return 1;
            resultado = executar_prioridade_sem(processos, quantidade);
            break;
        case 5:
            printf("\n===== Prioridade com preempcao =====\n");
            processos = carregar_processos(arquivo, &quantidade);
            if (processos == NULL) return 1;
            resultado = executar_prioridade_com(processos, quantidade);
            break;
        case 6:
            printf("\n===== Round-Robin =====\n");
            processos = carregar_processos(arquivo, &quantidade);
            if (processos == NULL) return 1;
            resultado = executar_round_robin(processos, quantidade, config);
            break;
        case 7:
            printf("\n===== Round-Robin com prioridade e envelhecimento =====\n");
            processos = carregar_processos(arquivo, &quantidade);
            if (processos == NULL) return 1;
            resultado = executar_round_robin_prioridade_envelhecimento(processos, quantidade, config);
            break;
        default:
            printf("\nOpcao invalida.\n");
            return 1;
    }

    printf("\nQuantum: %d\n", config.quantum);
    printf("Aging: %d\n", config.aging);
    mostrar_resultado(resultado, processos, quantidade);
    free(processos);
    return 0;
}


int main(int argc, char *argv[]) {
    srand(time(NULL));

    /*
        Modo interface / linha de comando:
            programa <opcao> <arquivo>
        Ex.: programa 1 entrada.txt
    */
    if (argc >= 3) {
        return executar_opcao(atoi(argv[1]), argv[2]);
    }

    /*
        Modo menu no terminal (como era antes).
    */
    int opcao;

    printf("===== ESCALONADOR DE PROCESSOS =====\n\n");
    printf("1 - FCFS\n");
    printf("2 - SJF\n");
    printf("3 - SRTF\n");
    printf("4 - Prioridade sem preempcao\n");
    printf("5 - Prioridade com preempcao\n");
    printf("6 - Round-Robin\n");
    printf("7 - Round-Robin com prioridade e envelhecimento\n");

    printf("\nEscolha o algoritmo: ");
    scanf("%d", &opcao);

    /*
        No menu, cada algoritmo usa seu proprio arquivo de teste.
    */
    const char *arquivo;
    switch (opcao) {
        case 1: arquivo = "fcfs.txt"; break;
        case 2: arquivo = "sjf.txt"; break;
        case 3: arquivo = "srtf.txt"; break;
        case 4: arquivo = "prioridade_sem.txt"; break;
        case 5: arquivo = "prioridade_com.txt"; break;
        case 6: arquivo = "round_robin.txt"; break;
        case 7: arquivo = "round_robin_prioridade_envelhecimento.txt"; break;
        default:
            printf("\nOpcao invalida.\n");
            return 1;
    }

    return executar_opcao(opcao, arquivo);
}
