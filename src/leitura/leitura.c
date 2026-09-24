#include "leitura.h"

Processo *ler_processos(const char *nome_arquivo, int *quantidade) {
    FILE *arquivo = fopen(nome_arquivo, "r");

    if(arquivo == NULL) {
        return NULL;
    }

    int capacidade = 10;
    int qtd = 0;

    Processo *processos = malloc(capacidade * sizeof(Processo));

    if (processos == NULL) {
        fclose(arquivo);
        return NULL;
    }

    int criacao;
    int duracao;
    int prioridade;

    // enquanto houver uma linha contendo três inteiros válidos, leia outro processo
    while (fscanf(arquivo, "%d %d %d", &criacao, &duracao, &prioridade) == 3) {

        if (qtd == capacidade) {
            capacidade *= 2;

            Processo *temp = realloc(
                processos,
                capacidade * sizeof(Processo)
            );

            if(temp == NULL) {
                free(processos);
                fclose(arquivo);
                return NULL;
            }

            processos = temp;
        }

        processos[qtd].id = qtd + 1;
        processos[qtd].criacao = criacao;
        processos[qtd].duracao = duracao;
        processos[qtd].restante = duracao;
        processos[qtd].prioridade = prioridade;
        processos[qtd].inicio = -1;
        processos[qtd].fim = -1;
    
        qtd++;
    }

    fclose(arquivo);
    *quantidade = qtd;
    return processos;
};

Configuracao ler_configuracao(const char *nome_arquivo) {
    Configuracao config = {0, 0};

    FILE *arquivo = fopen(nome_arquivo, "r");

    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo de configuracao");
        return config;
    }

    char linha[100];

    // fgets pega uma linha inteira. Evita erros de leitura após quebra de linha.
    if (fgets(linha, sizeof(linha), arquivo) != NULL) {
        sscanf(linha, "quantum:%d", &config.quantum);
    }

    if (fgets(linha, sizeof(linha), arquivo) != NULL) {
        sscanf(linha, "aging:%d", &config.aging);
    }

    fclose(arquivo);

    return config;
}