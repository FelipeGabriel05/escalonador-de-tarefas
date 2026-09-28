#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "leitura/leitura.h"
#include "escalonadores/fcfs.h"
#include "escalonadores/sjf.h"
#include "escalonadores/srtf.h"


Processo *carregar_processos(const char *nome_arquivo, int *quantidade) {

    Processo *processos = ler_processos(nome_arquivo, quantidade);
    if (processos == NULL) {
        printf("Erro ao ler processos do arquivo %s.\n", nome_arquivo);
        return NULL;
    }

    // Mostra os processos carregados.
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


/*
    Mostra os resultados de um algoritmo.
*/
void mostrar_resultado(Resultado resultado) {

    printf("\nTempo medio de turnaround: %.2f\n", resultado.tempo_medio_turnaround);
    printf("Tempo medio de espera: %.2f\n", resultado.tempo_medio_espera);
    printf("Tempo medio de resposta: %.2f\n", resultado.tempo_medio_resposta);
    printf("Trocas de contexto: %d\n", resultado.trocas_contexto);

    // Mostra o diagrama dos tempos de execução
    printf("\nDiagrama:\n");

    for (int i = 0; i < resultado.tamanho_execucao; i++) {

        if (resultado.execucao[i] == -1) {
            printf("Tempo %d: ocioso\n",i);
        }
        else {
            printf("Tempo %d: P%d\n",i,resultado.execucao[i]);
        }
    }
    free(resultado.execucao);
}


int main() {
    srand(time(NULL));


    /*
        Menu para escolher o algoritmo.
    */
    int opcao;

    printf("===== ESCALONADOR DE PROCESSOS =====\n\n");

    printf("1 - FCFS\n");
    printf("2 - SJF\n");
    printf("3 - SRTF\n");

    printf("\nEscolha o algoritmo: ");
    scanf("%d", &opcao);

    if (opcao < 1 || opcao > 3) {

        printf("\nOpcao invalida.\n");

        return 1;
    }


    int quantidade;
    Processo *processos = NULL;
    Configuracao config = ler_configuracao("../config.txt");
    Resultado resultado;


    /*
        Cada algoritmo utiliza seu próprio arquivo
        durante os testes.
    */
    switch (opcao) {

        case 1:
            printf("\n===== FCFS =====\n");
            processos = carregar_processos("../fcfs.txt",&quantidade);
            if (processos == NULL) {
                return 1;
            }

            resultado = executar_fcfs(processos, quantidade);
            break;
        case 2:

            printf("\n===== SJF =====\n");

            processos = carregar_processos("../sjf.txt", &quantidade);
            if (processos == NULL) {
                return 1;
            }

            resultado = executar_sjf(processos, quantidade);
            break;
        case 3:

            printf("\n===== SRTF =====\n");
            processos = carregar_processos("../srtf.txt", &quantidade);
            if (processos == NULL) {
                return 1;
            }

            resultado = executar_srtf(processos, quantidade);
            break;
    }


    printf("\nQuantum: %d\n", config.quantum);
    printf("Aging: %d\n", config.aging);
    mostrar_resultado(resultado);
    free(processos);

    return 0;
}