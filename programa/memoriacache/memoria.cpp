/* 
  Trabalho de Arquitetura de Computadores I
  Tema: Simulação de Memória 
  Copyright 2018 by Álvaro Barbosa, Lorena Tavares, Murilo Lopes, Rodrigo Herculano, William Coelho
*/

#include <iostream>
#include <cstdlib>
#include <math.h>
#include <bitset>
#include <cstring>
#include <fstream>

using namespace std;

class TACache { // classe utilizada para implementacao de uma TACache
    friend class SACache;
    private:
        int cap; // capacidade da TACache
        int wi; // indice de escrita seguindo ordem FIFO
        int* diretorio; // vetor utilizado para armazenamento de tags
        int** tabela; // matriz utilizada para armazenamento de dados
    public:
        int numLinhas;
        int tamLinha;
        TACache(int c = 0, int l = 0) { // construtor TACache
            wi = 0;
            cap = c;
            tamLinha = l;
            numLinhas = cap / tamLinha;
            diretorio = new int[numLinhas];
            tabela = new int*[numLinhas];
            for (int i = 0; i < numLinhas; ++i) {
                tabela[i] = new int[l / sizeof(int)];
            }
        }
        
        ~TACache() { // destrutor TACache
            for (int i = 0; i < numLinhas; ++i) {
                delete[] tabela[i];
            }
            delete[] tabela;
            delete[] diretorio;
        }

        TACache createTACache(int c, int l) { // metodo implementado para chamar o construtor da TACache
            if((pow(2, log2(c)) == c) and (pow(2, log2(l)) == l) and ((c % l) == 0)) {
                TACache tac(c, l);
                return tac;
            }
            cerr << "\n\nEntradas invalidas!\n\n";
            exit(EXIT_FAILURE); 
        }

        int getTACacheCapacity(TACache &tac) { // retorna a capacidade da TACache
            return tac.cap;
        }

        int getTACacheLineSize(TACache &tac) { // retorna o tamanho da linha da TACache
            return tac.tamLinha;
        }

        bool getTACacheData(TACache &tac, int address, int* value) { // busca um dado na TACache
            int tag = address >> int(log2(tac.tamLinha));
            int offset = (((address) & (tac.tamLinha - 1)) >> 2);
            bool hit = false;
            for (int i = 0; (i < tac.numLinhas) and (!hit); ++i) { // percorre o diretorio verificando se ha uma tag igual a tag procurada
                if (!(tac.diretorio[i] ^ tag)) {
                    hit = true;
                    *value = tabela[i][offset];
                }
            }
            return hit; // retorna se achou ou nao o dado buscado
        }

        void setTACacheLine(TACache &tac, int address, int* line) { // escreve uma linha na TACache
            int tag = address >> int(log2(tac.tamLinha));
            bool jaExiste = false;
            for (int i = 0; i < tac.numLinhas; ++i) { // percorre o diretorio verificando se ha uma tag igual a tag procurada
                if (!(tac.diretorio[i] ^ tag)) {
                    memcpy(tabela[i], line, tac.tamLinha);
                    jaExiste = true;
                }
            }
            if (!jaExiste) { // se nao existe, ela escreve a tag, depois a linha no endereco
                tac.diretorio[tac.wi] = tag;
                memcpy(tabela[tac.wi], line, tac.tamLinha);
				tac.wi = (tac.wi + 1) % tac.numLinhas; // wi recebe o proximo "indice" acessivel do vetor, seguindo a ordem FIFO
			}
        }

        bool setTACacheData(TACache &tac, int address, int value) { // escreve um dado na TACache
            int tag = address >> int(log2(tac.tamLinha));
            int offset = (((address) & (tac.tamLinha - 1)) >> 2);
            bool hit = false;
            for (int i = 0; (i < tac.numLinhas) and (!hit); ++i) { // percorre o diretorio verificando se ha uma tag igual a tag procurada
                if (!(tac.diretorio[i] ^ tag)) {
                    hit = true;
                    tac.tabela[i][offset] = value;
                }
            }
            return hit; // retorna se escreveu o dado no endereco ou nao
        }
        
        void imprimeTabela() { // imprime tabela TACache, para testes de depuracao
            for (int i = 0; i < numLinhas; ++i) {
                for (unsigned int j = 0; j < (tamLinha / (sizeof(int))); ++j) {
                    cout << tabela[i][j] << " ";
                }
                cout << endl;
            }
            cout << "\n\n";
        }
};

struct Conjunto { // estrutura utilizada para representar um conjunto
    int lookup;
    TACache* cache;
};

class SACache { // classe utilizada para implementacao de uma SACache
    friend class TACache;
    public:
        int numLinhas;
        int tamLinha;
        int capConjunto;
        int qtdConjuntos;
        int associatividade;
        Conjunto* conjuntos;
        int wiSet; // indice de escrita seguindo ordem FIFO no conjunto
        SACache() {
        }
        
        SACache(int c, int a, int l) { // construtor da SACache
            wiSet = 0;
            associatividade = a;
            tamLinha = l;
            capConjunto = (associatividade * tamLinha);
            qtdConjuntos = c / capConjunto;
            conjuntos = new Conjunto[qtdConjuntos];
            for (int i = 0; i < qtdConjuntos; i++) {
                conjuntos[i].cache = new TACache(capConjunto, l);
            }
        }
        
        ~SACache() { // destrutor da SACache
            delete[] conjuntos;
        }
        
        SACache createSACache(int c, int a, int l) { // metodo que chama o construtor da SACache se os parametros passados estiverem corretos
            if((pow(2, log2(c)) == c) and (pow(2, log2(a)) == a) and (pow(2, log2(l)) == l) and ((c % (a * l)) == 0)) {
                SACache sac(c, a, l);
                return sac;
            }
            cerr << "\n\nEntradas invalidas!\n\n"; // caso os parametros estejam incorretos
            exit(EXIT_FAILURE); 
        }
        
        int getSACacheCapacity(SACache &sac) { // retorna a capacidade da SACache
            return sac.qtdConjuntos * sac.capConjunto;
        }
        
        int getSACacheLineSize(SACache &sac) { // retorna o tamanho da linha da SACache
            return sac.tamLinha;
        }
        
        bool getSACacheData(SACache &sac, int address, int* value) { // busca um dado dentro da SACache
            int lookup;
            if (log2(sac.qtdConjuntos) == 0) {
                lookup = (address >> int(log2(sac.tamLinha)));
            }
            else {
                lookup = (address >> int(log2(sac.tamLinha))) % int(log2(sac.qtdConjuntos));
            }
            bool hit = false;
            for (int i = 0; (i < sac.qtdConjuntos) and (!hit); ++i) { // verifica se o lookup buscado e o mesmo do conjunto
                if (!(sac.conjuntos[i].lookup ^ lookup)) {
                    hit = sac.conjuntos[i].cache->getTACacheData(*sac.conjuntos[i].cache, address,value);
                }
            }
            return hit; // retorna um booleano que indica se foi encontrado
        }

        void setSACacheLine(SACache &sac, int address, int* line) { // escreve uma linha na SACache
            int lookup;
            if (log2(sac.qtdConjuntos) == 0) {
                lookup = (address >> int(log2(sac.tamLinha)));
            }
            else {
                lookup = (address >> int(log2(sac.tamLinha))) % int(log2(sac.qtdConjuntos));
            }
            bool jaExiste = false;
            for (int i = 0; (i < sac.qtdConjuntos and (!jaExiste)); ++i) { // verifica se o lookup buscado e o mesmo do conjunto
                if (!(sac.conjuntos[i].lookup ^ lookup)) {
                    jaExiste = true;
                    sac.conjuntos[i].cache->setTACacheLine(*sac.conjuntos[i].cache, address, line);
                }
            }
            if (!jaExiste) {
                sac.conjuntos[sac.wiSet].cache->setTACacheLine(*sac.conjuntos[sac.wiSet].cache, address, line);
                sac.conjuntos[wiSet].lookup = lookup;
				sac.wiSet = (sac.wiSet + 1) % sac.qtdConjuntos; // atualiza a variavel "wiSet" para seguir a ordem FIFO
			}
        }
        
        bool setSACacheData(SACache &sac, int address, int value) { // escreve um dado na SACache
            int lookup;
            if(log2(sac.qtdConjuntos) == 0){
                lookup = (address >> int(log2(sac.tamLinha)));
            }
            else{
                lookup = (address >> int(log2(sac.tamLinha))) % int(log2(sac.qtdConjuntos)) ;
            }
            bool hit = false;
            for (int i = 0; (i < sac.qtdConjuntos) and (!hit); ++i) { // verifica se o lookup buscado e o mesmo do conjunto
                if (!(sac.conjuntos[i].lookup ^ lookup)) {
                    hit = sac.conjuntos[i].cache->setTACacheData(*sac.conjuntos[i].cache, address, value);
                }
            }
            return hit; // retorna um booleano que indica se foi encontrado ou nao
        }
        
        SACache duplicateSACache(SACache &sac) { // metodo utilizado para duplicacao de uma SACache
            SACache sac2(sac.capConjunto * sac.qtdConjuntos, sac.associatividade,  sac.tamLinha);
            return sac2;
        }
};

class MainMemory { // classe utilizada para implementacao da MainMemory
    friend class Cache;
    public:
        int* vetor; // tam = ramsize + vmsize
        int ramTam;
        int virTam;
        MainMemory(int ramsize, int vmsize) { // construtor da MainMemory
            ramTam = ramsize;
            virTam = vmsize;
            vetor = new int[(ramTam + virTam) >> 2];
        }
        
        ~MainMemory() { // destrutor da MainMemory
            delete[] vetor;
        }
        
        MainMemory createMainMemory(int ramsize, int vmsize) { // metodo que chama o construtor
            MainMemory memoria(ramsize,vmsize);
            return memoria;
        }
        
        int getMainMemoryData(MainMemory &mem, int address, int* value) { // retorna -1 quando o valor nao foi encontrado, e 4 se foi
            for (int i = 0; i < ((ramTam + virTam) >> 2); ++i) {
                if (i == (address >> 2)) { // atribui a value que esta em vetor[i] se o endereço for valido
                    *value = vetor[i];
                    return 4;
                }
            }
            return -1;
        }
        int setMainMemoryData(MainMemory &mem, int address, int value) { // retorna -1 quando o valor nao foi escrito, e 4 se foi
            for (int i = 0; i < ((ramTam + virTam) >> 2); ++i) {
                if (i == (address >> 2)) {// atribui ao vetor[i] que esta em value se o endereço for valido
                    vetor[i] = value;
                    return 4;
                }
            }
            return -1;
        }
};


class Cache { // classe utilizada para implementacao da hierarquia de cache
    friend class SACache;
    friend class MainMemory;
    private:
    public:
        SACache* l1dCache;
        SACache* l1iCache;
        SACache* l2Cache;
        SACache* l3Cache;
    public:
        Cache(SACache &l1d, SACache &l1i, SACache &l2, SACache &l3) {// contrutor da Cache
            l1dCache = &l1d;
            l1iCache = &l1i;
            l2Cache = &l2;
            l3Cache = &l3;
        }
        
        Cache createCache(SACache &l1d, SACache &l1i, SACache &l2, SACache &l3) { // metodo que chama o construtor da Cache se os parametros passados estiverem corretos
            if ((l1d.tamLinha <= l2.tamLinha) and (l1i.tamLinha <= l2.tamLinha) and (l2.tamLinha < l3.tamLinha)) {
                Cache cache(l1d, l1i, l2, l3);
                return cache;
            }
            cerr << "\n\nEntradas invalidas!\n\n";
            exit(EXIT_FAILURE);
        }
        
        void fetchCacheData(Cache &sac, MainMemory &mmem, int address, int ret) { // atualiza as caches de acordo com a necessidade
            switch (ret) {
                case 2:
                    // atualiza l1 de dados
                    sac.l1dCache->setSACacheLine(*sac.l1dCache, address, ((int*)((long)mmem.vetor + (long)address)));
                    break;
                case 3:
                    // atualiza l1, l2
                    sac.l1dCache->setSACacheLine(*sac.l1dCache, address, ((int*)((long)mmem.vetor + (long)address)));
                    sac.l2Cache->setSACacheLine(*sac.l2Cache, address, ((int*)((long)mmem.vetor + (long)address)));
                    break;
                case 4:
                    // atualiza l1, l2, l3
                    sac.l1dCache->setSACacheLine(*sac.l1dCache, address, ((int*)((long)mmem.vetor + (long)address)));
                    sac.l2Cache->setSACacheLine(*sac.l2Cache, address, ((int*)((long)mmem.vetor + (long)address)));
                    sac.l3Cache->setSACacheLine(*sac.l3Cache, address, ((int*)((long)mmem.vetor + (long)address)));
                    break;
                case -1:
                    // endereco invalido, atualizacao das caches impossibilitada
                    break;
            }
        }
        
        void fetchCacheInstruction(Cache &sac, MainMemory &mmem, int address, int ret) { // atualiza as caches de acordo com a necessidade
            switch (ret) {
                case 2:
                    // atualiza l1 de instrucoes
                    sac.l1iCache->setSACacheLine(*sac.l1iCache, address, ((int*)((long)mmem.vetor + (long)address)));
                    break;
                case 3:
                    // atualiza l1, l2
                    sac.l1iCache->setSACacheLine(*sac.l1iCache, address, ((int*)((long)mmem.vetor + (long)address)));
                    sac.l2Cache->setSACacheLine(*sac.l2Cache, address, ((int*)((long)mmem.vetor + (long)address)));
                    break;
                case 4:
                    // atualiza l1, l2, l3
                    sac.l1iCache->setSACacheLine(*sac.l1iCache, address, ((int*)((long)mmem.vetor + (long)address)));
                    sac.l2Cache->setSACacheLine(*sac.l2Cache, address, ((int*)((long)mmem.vetor + (long)address)));
                    sac.l3Cache->setSACacheLine(*sac.l3Cache, address, ((int*)((long)mmem.vetor + (long)address)));
                    break;
                case -1:
                    // endereco invalido, atualizacao das caches impossibilitada
                    break;
            }
        }
        
        int getCacheData(Cache &c, MainMemory &mmem, int address, int* value) { // busca um dado seguindo a hierarquia de cache
            int ret;
            if (c.l1dCache->getSACacheData(*c.l1dCache, address, value)) { // encontrou dado na cache l1 de dados
                return 1;
            }
            else if (c.l2Cache->getSACacheData(*c.l2Cache, address, value)) { // encontrou dado na cache l2
                ret = 2;
            }
            else if (c.l3Cache->getSACacheData(*c.l3Cache, address, value)) { // encontrou dado na cache l3
                ret = 3;
            }
            else {
                ret = mmem.getMainMemoryData(mmem, address, value); // busca dado na memoria, retorna -1 se dado não está na memória, ou 4 quando o encontra
            }
            
            if (ret > 1) {
                c.fetchCacheData(c, mmem, address, ret); // chama este metodo para atualizacao das caches, caso necessario
            }
            return ret;
        }
        
        int getCacheInstruction(Cache &c, MainMemory &mmem,  int  address, int* value) { // busca uma instrucao seguindo a hierarquia de cache
            int ret;
            if (c.l1iCache->getSACacheData(*c.l1iCache, address, value)) { // encontrou instrucao em l1 de instrucoes
                return 1;
            }
            else if (c.l2Cache->getSACacheData(*c.l2Cache, address, value)) { // encontrou instrucao em l2
                ret = 2;
            }
            else if (c.l3Cache->getSACacheData(*c.l3Cache, address, value)) { // encontrou instrucao em l3
                ret = 3;
            }
            else {
                ret = mmem.getMainMemoryData(mmem, address, value); // buscou instrucao na memória, retorna -1 se instrucao não está na memória, ou 4 quando a encontra
            }
            
            if (ret > 1) {
                c.fetchCacheInstruction(c, mmem, address, ret); // chama este metodo para atualizacao das caches, caso necessario
            }
            return ret;
        }
        
        int setCacheData(Cache &c, MainMemory &mmem, int address, int value) { // escreve dados em todos os níveis da cache
            int ret;
            if (c.l1dCache->setSACacheData(*c.l1dCache, address, value)) { // achou endereco para escrever o dado em l1 de dados
                ret = 1;
            }
            else if (c.l2Cache->setSACacheData(*c.l2Cache, address, value)) { // achou endereco para escrever o dado em l2
                ret = 2;
            }
            else if (c.l3Cache->setSACacheData(*c.l3Cache, address, value)) { // achou endereco para escrever o dado em l3
                ret = 3;
            }
            else {
                ret = mmem.setMainMemoryData(mmem, address, value); // busca dado na memoria, retorna -1 se dado não está na memória, ou 4 quando o encontra
            }
            
            if (ret > 1) {
                c.fetchCacheData(c,mmem,address, ret); // chama este metodo para atualizacao das caches, caso necessario
            }
            c.l1dCache->setSACacheData(*c.l1dCache, address, value);
            c.l2Cache->setSACacheData(*c.l2Cache, address, value);
            c.l3Cache->setSACacheData(*c.l3Cache, address, value);
            mmem.setMainMemoryData(mmem, address, value);
            return ret;
        }
        
        int setCacheInstruction(Cache &c, MainMemory &mmem, int address, int value) { // escreve instruções em  todos os níveis da cache
            int ret;
            if (c.l1iCache->setSACacheData(*c.l1iCache, address, value)) { // achou endereco para escrever a instrucao em l1 de instrucoes
                ret = 1;
            }
            else if (c.l2Cache->setSACacheData(*c.l2Cache, address, value)) { // achou endereco para escrever a instrucao em l2
                ret = 2;
            }
            else if (c.l3Cache->setSACacheData(*c.l3Cache, address, value)) { // achou endereco para escrever a instrucao em l3
                ret = 3;
            }
            else {
                ret = mmem.setMainMemoryData(mmem, address, value); // buscou instrucao na memória, retorna -1 se instrucao não está na memória, ou 4 quando a encontra
            }
            
            if (ret > 1) {
                c.fetchCacheInstruction(c,mmem,address, ret); // chama este metodo para atualizacao das caches, caso necessario
            }
            c.l1dCache->setSACacheData(*c.l1dCache, address, value);
            c.l2Cache->setSACacheData(*c.l2Cache, address, value);
            c.l3Cache->setSACacheData(*c.l3Cache, address, value);
            mmem.setMainMemoryData(mmem, address, value);
            return ret;
        }

        Cache* duplicateCache(Cache &c){ // metodo utilizado para duplicacao da hierarquia de cache
            SACache* cl1d = new SACache();
            SACache* cl1i = new SACache();
            SACache* cl2 = new SACache();
            *cl1d = c.l1dCache->duplicateSACache(*c.l1dCache);
            *cl1i = c.l1iCache->duplicateSACache(*c.l1iCache);
            *cl2 = c.l2Cache->duplicateSACache(*c.l2Cache);
            Cache* cacheDuplicada = new Cache(*cl1d, *cl1i, *cl2, *c.l3Cache);
            return cacheDuplicada;
        }
};

class Memory { // classe utilizada para implementacao da hierarquia de memoria
    public:
        Cache* cache;
        MainMemory* mainMemory;
        Memory(Cache &c, MainMemory &mem) { // construtor da Memory
            cache = &c;
            mainMemory = &mem;
        }
        
        Memory() {
        }
        
        Memory createMemory(Cache &c, MainMemory &mem) { // metodo que chama o construtor da Memory
            Memory memory(c,mem);
            return memory;
        }
        
        int getData(Memory &mem, int address, int* value) { // metodo que busca dado na hierarquia de memoria
            return mem.cache->getCacheData(*mem.cache, *mem.mainMemory,address, value);
        }
        
        int getInstruction(Memory &mem, int address, int* value) { // metodo que busca instrucao na hierarquia de memoria
            return mem.cache->getCacheInstruction(*mem.cache, *mem.mainMemory,address, value);
        }
        
        int setData(Memory &mem, MainMemory &mmem, int address, int value) { // metodo que escreve dado na hierarquia de memoria
            int ret = mem.cache->setCacheData(*mem.cache, mmem, address, value);
            mem.mainMemory->setMainMemoryData(*mem.mainMemory, address, value);
            return ret;
        }
        
        int setShortData(Memory &mem, int address, unsigned short value) { // metodo que escreve dado na hierarquia de memoria
            int i;
            int valInt = (int)value;
           
            int ret = getData(mem,address,&i);
            
            // setar a metade correta 
            if ((address & 0x3) == 0) {
                valInt = valInt << 16;
                i = (i & 0x0000FFFF) | valInt;
            } else {
                i = (i & 0xFFFF0000) | valInt;
            }
            
            mem.cache->setCacheData(*mem.cache, *mem.mainMemory, address, i);
            mem.mainMemory->setMainMemoryData(*mem.mainMemory, address, i);
            return ret;
        }
        
        int setInstruction(Memory &mem, MainMemory &mmem, int address, int value) { // metodo que escreve instrucao na hierarquia de memoria
            int ret = mem.cache->setCacheInstruction(*mem.cache, mmem, address, value);
            mem.mainMemory->setMainMemoryData(*mem.mainMemory, address, value);
            return ret;
        }
        
        Memory duplicateMemory(Memory &mem) { // duplica a Memory
            Cache* cacheDuplicada = mem.cache->duplicateCache(*mem.cache);
            Memory mem2(*cacheDuplicada, *mem.mainMemory);
            return mem2; // retorna a memoria duplicada
        }
};

class Processor { // classe utilizada para implementacao do processador
    public:
        int qtdNucleos;
        Memory* memory;
        Processor() {
        }
        
        Processor(Memory &mem, int ncores) { // construtor do Processor
            qtdNucleos = ncores;
            memory = new Memory[qtdNucleos];
            memory[0] = mem; // atribui a hierarquia de memoria ao primeiro nucleo
            for (int i = 1; i < qtdNucleos; ++i) { 
                memory[i] = mem.duplicateMemory(mem); // atribui a duplicata da hierarquia de memoria a cada nucleo adicional
            }
        }
        
        Processor createProcessor(Memory &mem, int ncores) { // metodo que chama o construtor do Processor
            Processor proc(mem, ncores);
            return proc;
        }
};

unsigned short converterParaShort(unsigned short endereco, int value) {
  unsigned short ret;
  if((endereco & 0x3) != 0) { 
    // se o endereco for multiplo de 4
    // pega os 16 bits mais significativos
    ret = (unsigned short) ((value >> 16) & 0xFFFF);
  } else {
    // senao, pega os 16 bits menos significativos
    ret = (unsigned short) (value & 0xFFFF);
  }
  return ret;
}
