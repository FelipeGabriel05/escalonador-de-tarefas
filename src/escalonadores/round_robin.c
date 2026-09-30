#include "round_robin.h"

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

// Round-Robin sem prioridade

// Cria a estrutura de fila
// Fila circular de índices do vetor "copia"
typedef struct {
    int *itens; // vetor que armazena os índices dos processos
    int capacidade; // quantidade máxima de elementos que a fila pode armazenar = quantidade de processos
    int cabeca; // indica onde está o primeiro elemento da fila
    int tamanho; // indica quantos elementos existem atualmente na fila
} Fila;
//Cria uma fila com determinada capacidade
static Fila fila_criar(int capacidade) {
    Fila fila;

    fila.itens = malloc(capacidade * sizeof(int)); // reserva memória para armazenar os índices dos processos
    // Inicializa a fila vazia
    fila.capacidade = capacidade;
    fila.cabeca = 0;
    fila.tamanho = 0;

    return fila;
}

// Insere um processo no final da fila
static void fila_inserir(Fila *fila, int indice) {
    fila->itens[(fila->cabeca + fila->tamanho) % fila->capacidade] = indice;
    fila->tamanho++;
}

// Retorna o processo que estava na frente
static int fila_remover(Fila *fila) {
    int indice = fila->itens[fila->cabeca]; // pega o primeiro processo

    fila->cabeca = (fila->cabeca + 1) % fila->capacidade; // depois move a cabeça
    fila->tamanho--; // e diminui o tamanho

    return indice;
}

// Cria um vetor de índices ordenados por instante de criação do processo
static int *ordenar_por_chegada(const Processo *processos, int quantidade) {
    int *ordem = malloc(quantidade * sizeof(int));

    if (ordem == NULL) {
        return NULL;
    }

    for (int i = 0; i < quantidade; i++) {
        ordem[i] = i; // Começa na ordem original (ainda não ordenada)
    }

    // Insertion sort: ordena "ordem" pelo instante de criação de cada processo
    for (int i = 1; i < quantidade; i++) {
        int chave = ordem[i]; // Elemento que vamos posicionar no lugar certo
        int j = i - 1;

        // Empurra pra frente todo elemento com criação maior que o da chave
        while (j >= 0 && processos[ordem[j]].criacao > processos[chave].criacao) {
            ordem[j + 1] = ordem[j];
            j--;
        }

        ordem[j + 1] = chave; // Encaixa a chave no espaço que sobrou
    }

    return ordem;
}

// Adiciona na fila os processos que ainda não entraram e já chegaram
static void enfileirar_chegadas(
    Fila *fila,
    const Processo *copia,
    const int *ordem,
    int quantidade,
    int *proximo, // ponteiro: precisa alterar essa variável do chamador entre uma chamada e outra
    int tempo
) {
    while (*proximo < quantidade && copia[ordem[*proximo]].criacao <= tempo) {
        int indice = ordem[*proximo]; // Pega o índice do próximo processo

        if (copia[indice].restante > 0) {
            fila_inserir(fila, indice); // Se ele ainda precisa executar, coloca na fila
        }

        (*proximo)++;
    }
}

// Executa o Round Robin
Resultado executar_round_robin(Processo *processos, int quantidade, Configuracao config) {
    Resultado resultado = resultado_vazio();

    if (quantidade <= 0 || config.quantum <= 0) { // Se não há processos ou o quantum é inválido, encerra
        return resultado;
    }

    Processo *copia = copiar_processos(processos, quantidade); // Cria a cópia
    int *ordem = ordenar_por_chegada(processos, quantidade); // Cria a ordem dos processos por chegada
    Fila fila = fila_criar(quantidade); // Cria a fila do Round Robin
    int *execucao = malloc((calcular_tempo_maximo(processos, quantidade) + 1) * sizeof(int)); // Cria vetor de que processo está usando a cpu em cada instante
    // Se algum malloc falhou, libera o que já foi alocado (free em NULL não faz nada) e devolve vazio
    if (copia == NULL || ordem == NULL || fila.itens == NULL || execucao == NULL) {
        free(copia);
        free(ordem);
        free(fila.itens);
        free(execucao);
        return resultado;
    }

    resultado.execucao = execucao;

    int tempo = 0; // Relógio do sistema
    int proximo = 0; // Indica qual é o próximo processo que ainda precisa ser colocado na fila
    int ultimo = -1; // Guarda o último processo executado. Começa em -1, pois nenhum processo foi executado ainda
    int finalizados = finalizar_processos_vazios(copia, quantidade); // Conta os processos que já estavam terminados

    enfileirar_chegadas(&fila, copia, ordem, quantidade, &proximo, tempo); // Coloca na fila todos os processos que já chegaram no tempo atual

// Enquanto ainda houver processo não terminado, continue
    while (finalizados < quantidade) {

        if (fila.tamanho == 0) { // Caso a fila esteja vazia, não há processo pronto para executar agora
            resultado.execucao[resultado.tamanho_execucao] = -1; // Registra cpu ociosa
            resultado.tamanho_execucao++; 

            tempo++; // Passa 1 segundo
            enfileirar_chegadas(&fila, copia, ordem, quantidade, &proximo, tempo); // Verifica se algum processo chegou
            continue;
        }

        int atual = fila_remover(&fila); // Pega o primeiro processo da fila

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

        enfileirar_chegadas(&fila, copia, ordem, quantidade, &proximo, tempo);

        if (copia[atual].restante == 0) { // Depois verifica se o processo terminou
            copia[atual].fim = tempo; // Se terminou, registra o tempo de fim e aumenta o número de processos finalizados
            finalizados++;
        } else {
            fila_inserir(&fila, atual); // Se não terminou, coloca o processo novamente no final da fila
        }
    }

    calcular_metricas(copia, quantidade, &resultado);

    free(copia);
    free(ordem);
    free(fila.itens);

    return resultado;
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
Resultado executar_round_robin_prioridade(Processo *processos, int quantidade, Configuracao config) {
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
