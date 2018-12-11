# trabalho-arquitetura-i

Arquivos do trabalho de arquitetura do professor André Saúde, para a disciplina de Arquitetura de Computadores.

## Grupo
* Guilherme Barbosa Ochikubo
* Guilherme Henrique de Melo
* Leonardo Henrique de Braz

## Detalhes
Este trabalho possui um analisador de execução a partir dos parâmetros informados. Ele foi feito para receber o programa responsável por ser o simulador tARM, utilizando uma hierarquia de cache inicializável e também um conjunto de instruções compiladas em linguagem de máquina para simulação de uma arquitetura 16 bits. 

## Utilização
Para utilizar o simulador, basta:
* Possuir o executável do simulador tARM;
* Possuir os arquivos de hierarquia de cache (são necessários dois para comparações);
* Possuir o arquivo de instruções

Para executar:
* Compilar o representante ao simulador tARM (dentro da pasta **programa**): ` g++ programa/simulador-arm.cpp -o path-executavel-arm`;
* Compilar o avaliador de performance: `g++ avaliador-performance path-avaliador`
* Executar o programa: `./path-avaliador path-executavel-arm path-hierarquia1 path-hierarquia2 path-instrucoes`

## Problema/objetivo
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

## Problemas encontrados

### Endereçamento (por André Vital Saúde)

Caros alunos, 

Temos um problema de endereçamento quando vamos integrar o tARM com o SHM: o tARM é 16 bits e o SHM é 32 bits. Para resolver esta questão, explico com base no estágio de busca da instrução.

O estágio de busca da instrução no tARM original é implementado desta forma:

<pre>
<code>//Funcao de Busca da Instrucao
void BuscaInstrucao(Memory m, int v[]){
    IR = mem[PC>>1];
}</code></pre>

O índice da memória é PC>>1 (divisão inteira de PC por 2) porque a memória é um vetor de shorts (tamanho da palavra de um computador 16 bits) e o PC é o endereço em bytes. Veja a seguinte figura:

PC é um endereço em bytes, mas sempre múltiplo de 2, porque o tARM é 16 bits. A memória é um vetor de short, por isso o acesso é feito no índice PC/2.

O SHM, por sua vez, está implementado para palavras de 32 bits. Portanto, conforme figura, ao ler o endereço 6, em bytes, o SHM retornará todo o inteiro de índice 1, ou seja, os 4 bytes de endereços 4, 5, 6 e 7. Ao ler o endereço 4, o mesmo inteiro de índice 1 é retornado. 

Pois bem, se você quer ler o endereço 4, você quer na verdade os bytes 4 e 5, e se você quer ler o endereço 6, você quer os bytes 6 e 7. Portanto, se o endereço lido for um múltiplo de 4, você deve pegar os 16 bits mais significativos do valor retornado, e se o endereço lido não for múltiplo de 4, você deve pegar os 16 bits menos significativos.

Estas considerações servem para a busca da instrução. Para acesso a dados, tudo dependeria do tipo da variável acessada, mas só consideraremos o tipo short, então as considerações acima serão assumidas também para acesso a dados.

O trecho a seguir transforma o estágio de busca da instrução para o formato integrado. A função getShort poderá ser usada também para o acesso a dados.

<pre><code>//Funcao de Busca da Instrucao
void iFetch(Memory m, int hits[]){
    int inteiro;
    hits[m.getInstruction(m,PC,&inteiro)]++; // contabilização dos hits
    IR = getShort(PC, inteiro); // pega o short correto
}
</code></pre>

<pre><code> 
unsigned short getShort(unsigned short address, int value) {
  unsigned short ret;
  if((address & 0x3) == 0) { 
    // se o endereco for multiplo de 4
    // pega os 16 bits mais significativos
    ret = (unsigned short) ((value >> 16) & 0xFFFF);
  } else {
    // senao, pega os 16 bits menos significativos
    ret = (unsigned short) (value & 0xFFFF);
  }
  return ret;
}
</code></pre>