#include "fcfs.h"
#include <stdlib.h>

/*
    Escolhe qual processo deverá receber a CPU.
    No FCFS (First Come, First Served), o processo escolhido
    é aquele que chegou primeiro.
    Caso dois processos tenham o mesmo instante de criação,
    usamos o menor ID como critério de desempate.
    A função retorna o índice do processo escolhido no vetor.
    Caso nenhum processo esteja disponível naquele momento,
    retorna -1.
 */

static int escolher_processo(Processo *processos, int quantidade, int tempo) {

    int escolhido = -1;

    /* 
        Percorremos todos os processos procurando aqueles
        que já podem executar.
    */

    for (int i = 0; i < quantidade; i++) {

        /*
            Se o tempo restante é 0, significa que o processo já terminou 
            e não pode ser escolhido novamente.
        */
        if (processos[i].restante <= 0) {
            continue;
        }

        /*
            Se o processo ainda não chegou, ele não está
            disponível para utilizar a CPU.
        */
        if (processos[i].criacao > tempo) {
            continue;
        }

        /*
            Se ainda não escolhemos nenhum processo,
            o primeiro processo disponível passa a ser
            o candidato.
        */
        if (escolhido == -1) {
            escolhido = i;
            continue;
        }

        /*
            Primeiro critério: No FCFS, quem chegou primeiro deve executar primeiro.
        */
        if (processos[i].criacao < processos[escolhido].criacao) {
            escolhido = i;
        } else if (
            processos[i].criacao == processos[escolhido].criacao &&
            processos[i].id < processos[escolhido].id
        ) 
        {
            /*
                Segundo critério: Se os dois chegaram no mesmo instante, 
                escolhemos o processo com menor ID.
            */
            escolhido = i;
        }
    }

    return escolhido;
}


/*
    Executa o algoritmo FCFS.
 
    Recebe:
      processos  -> vetor com os processos lidos da entrada
      quantidade -> quantidade de processos
 
    Retorna:
      Resultado contendo as métricas da execução,
      número de trocas de contexto e o diagrama.
 */
Resultado executar_fcfs(Processo *processos, int quantidade) {

    Resultado resultado;


    //Inicializamos o resultado.
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
        Isso é importante porque durante a simulação vamos
        modificar informações como "restante", "inicio" e "fim".
        Não queremos modificar o vetor original que foi lido
        do arquivo, pois posteriormente os mesmos processos
        serão utilizados pelos outros algoritmos.
    */
    Processo *copia = malloc(quantidade * sizeof(Processo));

    if (copia == NULL) {
        return resultado;
    }


    //Copiamos cada processo para o novo vetor.
    for (int i = 0; i < quantidade; i++) {
        copia[i] = processos[i];
    }


    int tempo_total = 0;
    for (int i = 0; i < quantidade; i++) {
        tempo_total += copia[i].duracao;

        if (copia[i].criacao > tempo_total) {
            tempo_total = copia[i].criacao;
        }
    }
    tempo_total += 1;
    resultado.execucao = malloc(tempo_total * sizeof(int));


    if (resultado.execucao == NULL) {
        free(copia);
        return resultado;
    }


    /*
        "tempo" representa o relógio da simulação.
        Começamos no instante 0.
    */
    int tempo = 0;


    /*
        Quantidade de processos que já terminaram.
        Quando todos terminarem, a simulação acaba.
    */
    int finalizados = 0;


    /*
        Índice do processo que está atualmente usando a CPU.
        -1 significa que nenhum processo está executando.
     */
    int processo_atual = -1;


    /*
        Guarda o processo que estava usando a CPU anteriormente.
        É utilizado para contabilizar trocas de contexto.
    */
    int processo_anterior = -1;


    /*
        A simulação continua enquanto existir pelo menos
        um processo que ainda não terminou.
    */
    while (finalizados < quantidade) {


        /*
            Se não existe processo executando atualmente,
            precisamos escolher um.
        */
        if (processo_atual == -1) {

            processo_atual = escolher_processo(copia, quantidade, tempo);


            /*
                Se nenhum processo foi encontrado, significa
                que ainda não chegou nenhum processo.
                Nesse caso a CPU fica ociosa durante 1 segundo.
                Usamos -1 para representar a CPU ociosa.
             */
            if (processo_atual == -1) {

                resultado.execucao[resultado.tamanho_execucao] = -1;

                resultado.tamanho_execucao++;

                tempo++;

                continue;
            }


            /*
                Se esse é o primeiro momento em que o processo
                recebe a CPU, registramos seu tempo de resposta.
                Exemplo:
                    processo chegou em 3
                      começou em 7
                resposta = 7 - 3 = 4
            */
            if (copia[processo_atual].inicio == -1) {
                copia[processo_atual].inicio = tempo;
            }


            /*
                Se já existia um processo executando e agora
                outro processo vai assumir a CPU, ocorreu
                uma troca de contexto.
            */
            if (
                processo_anterior != -1 &&
                processo_anterior != processo_atual
            ) {
                resultado.trocas_contexto++;
            }
        }


        /*
            Executamos o processo atual durante 1 segundo.
            Guardamos o ID do processo no diagrama.
        */
        resultado.execucao[resultado.tamanho_execucao] = copia[processo_atual].id;
        resultado.tamanho_execucao++;

        /*
            Como passou 1 segundo de execução, diminuímos
            o tempo restante do processo.
        */
        copia[processo_atual].restante--;


        // Avançamos o relógio da simulação em 1 segundo.
        tempo++;


        /*
         * Se o tempo restante chegou a 0,
         * o processo terminou.
         */
        if (copia[processo_atual].restante == 0) {

            /*
                "tempo" já foi incrementado.
                Portanto, se P1 começou em 0 e executou
                durante 5 segundos:
                início = 0
                fim = 5
             */
            copia[processo_atual].fim = tempo;

            // Aumentamos a quantidade de processos finalizados.
            finalizados++;

            //Guardamos qual processo acabou de utilizar a CPU.
            processo_anterior = processo_atual;


            //A CPU ficará livre e, na próxima iteração, escolheremos outro processo.
            processo_atual = -1;
        }
    }

    int soma_turnaround = 0;
    int soma_espera = 0;
    int soma_resposta = 0;


    for (int i = 0; i < quantidade; i++) {
        // Turnaround = tempo de término - tempo de criação
        int turnaround = copia[i].fim - copia[i].criacao;

        // Tempo de espera = turnaround - tempo de execução
        int espera = turnaround - copia[i].duracao;

        //Tempo de resposta = primeiro instante em que recebeu CPU - instante de criação
        int resposta = copia[i].inicio - copia[i].criacao;


        soma_turnaround += turnaround;
        soma_espera += espera;
        soma_resposta += resposta;
    }


    // Cálculo das médias
    resultado.tempo_medio_turnaround = (double)soma_turnaround / quantidade;
    resultado.tempo_medio_espera = (double)soma_espera / quantidade;
    resultado.tempo_medio_resposta = (double)soma_resposta / quantidade;

    free(copia);
    return resultado;
}