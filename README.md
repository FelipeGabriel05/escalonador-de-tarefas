# Simulador de Escalonamento — versão local

Interface web + sua lógica em C rodando de verdade. Um servidorzinho local
(`servidor.py`) liga a página ao seu programa em C: a página manda os dados,
o servidor roda o C e devolve o resultado.

A lógica dos escalonadores **não mudou**. O único ajuste foi no `main.c`, que
agora aceita rodar como `programa <opcao> <arquivo>` (além do menu de sempre no
terminal) e imprime uma linha `TIMELINE:` pra desenhar o gráfico.

## Como rodar

Precisa ter `python` e `gcc` instalados (você já tem o gcc, pois compilou o .exe).

1. Abra o terminal nesta pasta.
2. Rode:

   ```
   python servidor.py
   ```

   Na primeira vez ele compila o C sozinho.
3. Abra no navegador: **http://localhost:8000**
4. Escolha o algoritmo, digite os processos e clique em **Simular**.

Pra parar o servidor: `Ctrl+C`.

## Ainda dá pra usar pelo terminal

O menu antigo continua funcionando:

```
./programa          (Linux/Mac)
programa.exe        (Windows)
```

## Formato dos processos

Uma linha por processo: `criacao duracao prioridade`

```
0 5 2
0 2 3
1 4 1
3 3 4
```

## Arquivos

```
index.html      interface (Gantt + métricas)
servidor.py     servidor local que roda o C
c/              seu código C (main.c ajustado; resto intacto)
```
