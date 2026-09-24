#include <stdio.h>
#include <stdlib.h>
#include "leitura/leitura.h"

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
    free(processos);

    return 0;
}