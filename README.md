# trabalho-arquitetura-i

Arquivos do trabalho de arquitetura do professor André Saúde, para a disciplina de Arquitetura de Computadores.

## Enunciado
Objetivo: Juntar o trabalho de 2017/2 com o trabalho de 2018/1 para fazer um comparador de performance de dois computadores com distintas configurações de cache.

Sugestão de ordem de execução do trabalho:

1) Escolher um programa 2018/1 (simulador do ARM Thumb 16 bits - tARM) e rodá-lo com o arquivo summation.o (ver a pasta "Arquivos auxiliares 2018/1").
2) Escolher um programa 2017/2 (simulador de hierarquia de memória - SHM) e rodá-lo com o arquivo de teste caixa branca.
3) Integrar o SHM no tARM, eliminando a escrita de arquivos de saída do tARM e a leitura de arquivos de entrada do SHM, o que significa:
4) Trocar o tipo da variável global que representa a memória no tARM pelo tipo Memory do SHM
5) Copiar, do main do SHM para o main do tARM, o trecho que cria a hierarquia de memória (até o comando cmem)
6) Fazer o main do tARM agora trabalhar com dois arquivos: o arquivo de criação da cache e o arquivo com o código binário do programa, e contabilizar os hits na hierarquia, assim como faz o SHM.
7) Rodar o programa integrado com os arquivos hierarquia1.txt e summation.o, contabilizar os hits e calcular o tempo total de processamento (ver tempos de cada hit mais adiante).
8) Fazer o tARM rodar duas vezes um mesmo programa, uma vez para uma hierarquia de memória (hierarquia1.txt) e uma segunda vez para uma segunda hierarquia de memória (hierarquia2.txt), calcular os tempos e comparar a performance das duas hierarquias.
9) Estender estágio de decodificação da instrução para atender a todas as instruções presentes no programa summationfun.o, começando pelas instruções push e pop.

Considerar os seguintes tempos: 1, para hit em L1; 2, para hit em L2; 3, para hit em L3; e 10, para acesso à memória principal.
