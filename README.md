# Escalonador de Processos

Atividade prática da cadeira de Sistemas Operacionais, ministrada pelo professor Fernando Trinta, no curso de Ciência da Computação da UFC.

Simulador de escalonamento de processos que lê um conjunto de processos e simula diferentes algoritmos de escalonamento, mostrando as métricas e o diagrama de tempo de cada execução.

## Algoritmos implementados

- FCFS (First Come, First Served)
- SJF (Shortest Job First)
- SRTF (Shortest Remaining Time First)
- Prioridade sem preempção
- Prioridade com preempção
- Round-Robin
- Round-Robin com prioridade e envelhecimento (aging)

## Estrutura do projeto

```
escalonador-de-tarefas/
├── src/
│   ├── main.c              # menu e impressão dos resultados
│   ├── modelo/             # structs: Processo, Resultado, Configuracao
│   ├── leitura/            # leitura dos processos e da configuração
│   └── escalonadores/      # um par .c/.h por algoritmo
├── simulador.html          # interface visual (abre no navegador)
├── config.txt              # quantum e aging
└── *.txt                   # arquivos de entrada de cada algoritmo
```

## Como compilar

A partir da pasta `src`:

```
gcc escalonadores/*.c leitura/leitura.c main.c -o programa
```

## Como executar

```
./programa
```

No Windows (PowerShell), use `.\programa.exe`.

Ao rodar, escolha o algoritmo pelo número do menu. O programa lê os processos do arquivo `.txt` correspondente (que fica na pasta acima de `src`) e imprime o tempo médio de turnaround, o tempo médio de espera, o número de trocas de contexto e o diagrama de tempo.

## Formato da entrada

Cada linha de um arquivo de entrada é um processo, com três inteiros separados por espaço:

```
criacao duracao prioridade
```

Exemplo padrão: 

```
0 5 2
0 2 3
1 4 1
3 3 4
```

O arquivo `config.txt` define o quantum e a taxa de envelhecimento usados pelos Round-Robin:

```
quantum:2
aging:1
```

Obs.: a prioridade segue a convenção dos slides da disciplina — quanto maior o número, mais prioritário é o processo.

## Interface

O arquivo `simulador.html` é uma interface visual que roda direto no navegador (basta abrir o arquivo). Ela permite digitar os processos, escolher o algoritmo e ver o diagrama de tempo animado junto com as métricas.

## Equipe

- Felipe Gabriel — FCFS, SJF, SRTF
- Raissa Sousa — prioridade com e sem preempção, interface
- Ravena Marques — Round-Robin e Round-Robin com prioridade e aging
