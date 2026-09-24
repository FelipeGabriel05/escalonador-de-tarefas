#include <stdio.h>
#include <stdlib.h>
#include "leitura/leitura.h"
#include "escalonadores/fcfs.h"

int main() {
    int quantidade;

    Processo *processos = ler_processos("../processos.txt", &quantidade);
    Configuracao config = ler_configuracao("../config.txt");

    if (processos == NULL) {
        printf("Erro ao ler processos.\n");
        return 1;
    }
    printf("Quantidade: %d\n\n", quantidade);

    for (int i = 0; i < quantidade; i++) {
        printf(
            "P%d: criacao=%d duracao=%d prioridade=%d\n",
            processos[i].id,
            processos[i].criacao,
            processos[i].duracao,
            processos[i].prioridade
        );
    }

    printf("\nQuantum: %d\n", config.quantum);
    printf("Aging: %d\n", config.aging);
    // =========================================
    // EXECUÇÃO DO FCFS
    // =========================================

    Resultado resultado_fcfs = executar_fcfs(processos, quantidade);


    // =========================================
    // MOSTRA OS RESULTADOS
    // =========================================

    printf("\n===== FCFS =====\n");

    printf(
        "Tempo medio de turnaround: %.2f\n",
        resultado_fcfs.tempo_medio_turnaround
    );

    printf(
        "Tempo medio de espera: %.2f\n",
        resultado_fcfs.tempo_medio_espera
    );

    printf(
        "Tempo medio de resposta: %.2f\n",
        resultado_fcfs.tempo_medio_resposta
    );

    printf(
        "Trocas de contexto: %d\n",
        resultado_fcfs.trocas_contexto
    );


    // Mostra o diagrama
    printf("\nDiagrama:\n");

    for (int i = 0; i < resultado_fcfs.tamanho_execucao; i++) {

        if (resultado_fcfs.execucao[i] == -1) {
            printf("Tempo %d: ocioso\n", i);
        }
        else {
            printf(
                "Tempo %d: P%d\n",
                i,
                resultado_fcfs.execucao[i]
            );
        }
    }


    // Libera memória
    free(resultado_fcfs.execucao);
    free(processos);
    return 0;
}