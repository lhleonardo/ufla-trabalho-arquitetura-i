/* TRABALHO FINAL GCC117 - ARQUITETURA DE COMPUTADORES 1
    PROFESSOR: Andre Vital Saude
    GRUPO: Gabriel Braga Santos
           Eduardo Miranda Pedrosa Filho
           Joao Pedro de Almeida Andolpho
           Joao Pedro Fachini Alvarenga
*/

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "memoriacache/memoria.cpp"

using namespace std;

//Declaracao das variáveis globais
// unsigned short Mem[32768];
Memory* Mem;
unsigned short PC = 0x0070;
unsigned short IR;
unsigned short LR;
unsigned short SP = 32767;
unsigned short reg[8];
unsigned short A,B; 
unsigned short *D;
unsigned short result;
//Declaracao das variaveis de controle
bool pop = false;
bool branch = false;
bool Z = 0;
bool N = 0; 
int execode;
bool wb = false;
bool acabou = false;
//Declaracao e abertura do arquivo de saida
ofstream out("saida.txt");

// declaração das variáveis de hits
unsigned hitL1i = 0;
unsigned hitL1d = 0;
unsigned hitL2 = 0;
unsigned hitL3 = 0;
unsigned hitMemory = 0;
unsigned erros = 0;

unsigned short converterParaShort(unsigned short endereco, int value) {
  unsigned short ret;
  if((endereco & 0x3) == 0) { 
    // se o endereco for multiplo de 4
    // pega os 16 bits mais significativos
    ret = (unsigned short) ((value >> 16) & 0xFFFF);
  } else {
    // senao, pega os 16 bits menos significativos
    ret = (unsigned short) (value & 0xFFFF);
  }
  return ret;
}

//Funcao de Busca da Instrucao
void BuscaInstrucao(){
    // IR = Mem[PC>>1];
    int result;
    switch(Mem->getInstruction(*Mem, PC, &result)) {
        case 1:
            hitL1d++; break;
        case 2:
            hitL2++; break;
        case 3:
            hitL3++; break;
        case 4:
            hitMemory++; break;
        default:
            erros++; break;
    };
    IR = converterParaShort(PC, result);
    //Escrita no arquivo da leitura de instrucao
    out << "ri 0x" << std::hex << PC << endl;
}

//Funcao de Decodificacao da Instrucao
void Decodificacao(){
    //Switch para a deteccao dos 3 primeiros bits da instrucao
    switch(IR & 0xE000){
        case(0x0000):
            switch(IR & 0x1800){
                case(0x0000)://INSTRUCAO LSL
                    A = ((IR & 0x07C0) >> 6);
                    B = reg[(IR & 0x0038) >> 3];
                    D = &reg[(IR & 0x0007)];
                    wb = true;
                    execode = 5;
                    break;
                case(0x0800)://INSTRUCAO LSR
                    A = ((IR & 0x07C0) >> 6);
                    B = reg[(IR & 0x0038) >> 3];
                    D = &reg[(IR & 0x0007)];
                    wb = true;
                    execode = 6;
                    break;
            }
            switch(IR & 0x1E00){
                case(0x1800)://INSTRUCAO ADD PARA DOIS REGISTRADORES
                    B = reg[(IR & 0x01C0) >> 6]; 
                    A = reg[(IR & 0x0038) >> 3]; 
                    D = &reg[(IR & 0x0007)];
                    execode = 0;
                    wb = true;
                    break;
                case(0x1A00)://INSTRUCAO SUB PARA DOIS REGISTRADORES
                    B = reg[(IR & 0x01C0) >> 6]; 
                    A = reg[(IR & 0x0038) >> 3]; 
                    D = &reg[(IR & 0x0007)];
                    execode = 1;
                    wb = true;
                    break;
                case(0x1C00)://INSTRUCAO ADD PARA UM OFFSET DE 3 BITS
                    B = reg[(IR & 0x01C0) >> 6]; 
                    A = ((IR & 0x0038) >> 3); 
                    D = &reg[(IR & 0x0007)];
                    execode = 0;
                    wb = true;
                    break;
                case(0x1E00)://INSTRUCAO SUB PARA UM OFFSET DE 3 BITS
                    B = reg[(IR & 0x01C0) >> 6]; 
                    A = ((IR & 0x0038) >> 3); 
                    D = &reg[(IR & 0x0007)];
                    execode = 1;
                    wb = true;
                    break;
            }
            break;
        case(0x2000):
            switch(IR & 0x3800){
                case(0x2000)://INSTRUCAO MOV
                    A = (IR & 0x00FF);
                    execode = 2;
                    wb = true;
                    break;
                case(0x2800)://INSTRUCAO CMP
                    B = (IR & 0x00FF);
                    A = reg[(IR & 0x0700)>>8];
                    execode = 1;
                    wb = false;
                    break;
                case(0x3000)://INSTRUCAO ADD PARA UM OFFSET DE 8 BITS
                    D = &reg[(IR & 0x0700)>>8];
                    B = reg[(IR & 0x0700)>>8];
                    A = (IR & 0x00FF);
                    execode = 0;
                    wb = true;
                    break;
                case(0x3800)://INSTRUCAO SUB PARA UM OFFSET DE 8 BITS
                    D = &reg[(IR & 0x0700)>>8];
                    B = reg[(IR & 0x0700)>>8];
                    A = (IR & 0x00FF);
                    execode = 1;
                    wb = true;
                    break;
            }
        case(0x4000):
            switch(IR & 0x5E00){
                case(0x5600)://INSTRUCAO LDRH PARA DOIS REGISTRADORES
                    A = reg[(IR & 0x0038) >> 3];
                    B = reg[(IR & 0x01C0) >> 6];
                    D = &reg[(IR & 0x0007)];
                    execode = 4;
                    wb = true;
                    break;
            }
            switch(IR & 0x4C00){
                case(0x4800)://INSTRUCAO LDR RELACIONADA AO PC
                    D = &reg[(IR & 0x0700)>>8];
                    A = ((IR & 0x00FF) << 2);
                    B = PC;
                    execode = 4;
                    wb = true;
                    break;
                case(0x4600)://INSTRUCAO MOV HD RS
                    A = reg[(IR & 0x0038) >> 3];
                    D = &reg[(IR & 0x0007)];
                    execode = 2;
                    wb = true;
                    break;
            }
            break;
        case(0x6000):
            switch(IR & 0x7800){
                case(0x6000)://INSTRUCAO STR COM OFFSET DE 5 BITS
                    A = ((IR & 0x07C0)<<2);
                    B = reg[(IR & 0x0038)>>3];
                    D = &reg[(IR & 0x0007)];
                    wb = false;
                    execode = 3;
                    break;
                case(0x6800)://INSTRUCAO LDR COM OFFSET DE 5 BITS
                    A = ((IR & 0x07C0)<<2);
                    B = reg[(IR & 0x0038)>>3];
                    D = &reg[(IR & 0x0007)];
                    wb = true;
                    execode = 4;
                    break;
            }
            break;
        case(0x8000):
            switch(IR & 0x8800){
                case(0x8000)://INSTRUCAO STRH COM OFFSET DE 5 BITS
                    B = ((IR & 0x07C0) >> 6) << 1;
                    A = reg[(IR & 0x0038) >> 3];
                    D = &reg[(IR & 0x0007)];
                    wb = false;
                    execode = 3;
                    break;
                case(0x8800)://INSTRUCAO LDRH COM OFFSET DE 5 BITS
                    B = ((IR & 0x07C0) >> 6) << 1;
                    A = reg[(IR & 0x0038) >> 3];
                    D = &reg[(IR & 0x0007)];
                    wb = true;
                    execode = 4;
                    break;
            }
            break;
        case(0xA000):
            switch(IR & 0xBF00){
                case(0xBD00):// POP COM PC
                    acabou = true;  //caso a instrução seja POP de PC, parar a execução do programa
                    break;

                case (0xBC00): // pop sem pc
                    acabou = true;
                    break;
                case(0xB000):
                    switch(IR & 0xB080){
                        case(0xB000)://INSTRUCAO ADD RELACIONADA AO SP
                            A = SP;
                            B = (IR & 0x007F);
                            D = &SP;
                            execode = 0;
                            wb = true;
                            break;
                        case(0xB080)://INSTRUCAO SUB RELACIONADA AO SP
                            A = SP;
                            B = (IR & 0x007F);
                            D = &SP;
                            execode = 1;
                            wb = true;
                            break;
                    }
                case(0xA800)://INSTRUCAO ADD USANDO O SP E OUTRO REG
                    A = ((IR & 0x00FF) << 2);
                    B = SP;
                    D = &reg[(IR & 0x0700) >> 8];
                    wb = true;
                    execode = 0;
                    break;
                default://CASO PARA IGNORAR AS PRÓXIMAS PARTES DA EXECUCAO
                    //Este caso e verdadeiro para qualquer instrucao
                    //push ou pop que nao seja a do termino do programa
                    pop = true;
                    break;
            }
            break;
        case(0xC000):
            switch(IR & 0xDF80){//INSTRUCAO BLE
                //Decidimos dividir a decodificacao entre offset positivo
                //e offset negativo, gerando dois casos
                case(0xDD00):  //BLE para offset positivo
                    A = ((IR & 0x00FF) << 1);
                    execode = 8;
                    wb = false;
                    break;
                case(0xDD80):  //BLE para offset negativo
                    {
                        signed short l = ((IR & 0x00FF) << 1);
                        signed short aneg = 0xFE00 | l;
                        A = -aneg;
                        execode = 9;
                        wb = false;
                        break;
                    }
            }
            break;
        case(0xE000)://INSTRUCAO B(branch)
            //Decidimos dividir a decodificacao ente offset positivo
            //e offset negativo
            {
                //BRANCH COM OFFEST NEGATIVO
                signed short aux = ((IR & 0x07FF) << 1);
                if(aux < 0){
                    signed short aneg = 0xF000 | aux;
                    A = -aneg;
                    execode = 10;
                } else{
                    //BRANCH COM OFFEST POSITIVO
                    A = aux;
                    execode = 7;
                }
                wb = false;
                branch = true;
            }
            break;
        default://Caso o programa leia uma instrucao não implementada,
                //a execucao é encerrada
            acabou = true;
            break;
        
    }
}

//Funcao de Execucao
void ExeMem() {
    switch (execode) {
        case 0://ADD
            result = A + B;
            N = (int(A)+int(B)) < 0;
            Z = (int(A)+int(B)) == 0;
            break;
        case 1://SUB,CMP
            result = A - B;
            N = ((int(A)-int(B)) < 0);
            Z = (int(A)-int(B) == 0);
            break;
        case 2://MOV
            result = A;
            N = int(A) < 0;
            Z = int(A) == 0;
            break;
        case 3://STR, STRH
            result = (A+B);
            // Mem[result>>1] = *D;
            switch(Mem->setData(*Mem, *Mem->mainMemory, result << 1, *D)) {
                case 1:
                    cout << "hit de l1d ao realizar STR, STRH" << endl;
                    hitL1d++; break;
                case 2:
                    hitL2++; break;
                case 3:
                    hitL3++; break;
                case 4:
                    hitMemory++; break;
                default:
                    erros++; break;
            };
            out << "wd 0x" << std::hex << result << endl;
            break;
        case 4://LDR, LDRH
            // leitura da memória em um int, pela utilização
            // no trabalho de 20172 e conversão para short
            // após a busca
            int resultado;
            switch(Mem->getData(*Mem, (A+B) << 1, &resultado)) {
                case 1:
                    cout << "hit em L1d de LDR" << endl;
                    hitL1d++; break;
                case 2:
                    hitL2++; break;
                case 3:
                    hitL3++; break;
                case 4:
                    hitMemory++; break;
                default:
                    erros++; break;
            };
            result = short(resultado);
            // result = Mem[((A+B)>>1)];
            out << "rd 0x" << std::hex << (A+B) << endl;
            break;
        case 5://LSL
            result = B << A;
            N = ((int(B) << int(A)) < 0);
            Z = ((int(B) <<int(A)) == 0);
            break;
        case 6://LSR
            result = B >> A;
            N = ((int(B) >> int(A)) < 0);
            Z = ((int(B) >> int(A)) == 0);
            break;
        case 7://B COM OFFSET POSITIVO
            PC = PC + A + 4;
            break;
        case 8://BLE COM OFFSET POSITIVO
            if(Z or N){
                branch = true;
                PC = PC + A + 4;
            }
            break;
        case 9://BLE COM OFFSET NEGATIVO
            if(Z or N){
                branch = true;
                PC = PC - A + 4;
            }
            break;
        case 10://B COM OFFSET NEGATIVO
            PC = PC - A + 4;
            break;
    }
}

//Funcao de Write Back
void EscritaRegistrador() {
    if(wb){
        *D = result;
        wb = false;
    }
}

//Programa Principal
int main() {
    //Limpando o terminal para a execucao do programa
    system("clear");
    system("clear");
    cout << "Escreva o nome do arquivo de simulação: ";
    string nomeArq, nomeArqMem;

    cin >> nomeArq;
    cout << "Escreva o nome do arquivo de hierarquia de memória:";

    cin >> nomeArqMem;

    // variaveis necessarias para a execucao do arquivo
    SACache *l1d, *l1i, *l2, *l3;
    MainMemory *mp;
    Cache *cache;
    
    Processor *processador;

    ifstream arqMem(nomeArqMem);
    string comando;
    arqMem >> comando;
    int c, a, l, ramsize, vmsize, n;
    
    int aux = 0;
    if (comando == "cl1d") {
        arqMem >> c >> a >> l;
        l1d = new SACache(c, a, l);
        ++aux;
    }
    
    arqMem >> comando;
    if ((comando == "cl1i") and (aux == 1)) {
        arqMem >> c >> a >> l;
        l1i = new SACache(c, a, l);
        ++aux;
    }
    
    arqMem >> comando;
    if ((comando == "cl2") and (aux == 2)) {
        arqMem >> c >> a >> l;
        l2 = new SACache(c, a, l);
        ++aux;
    }
    
    arqMem >> comando;
    if ((comando == "cl3") and (aux == 3)) {
        arqMem >> c >> a >> l;
        l3 = new SACache(c, a, l);
        ++aux;
    }
    
    arqMem >> comando;
    if ((comando == "cmp") and (aux == 4)) {
        arqMem >> ramsize >> vmsize;
        mp = new MainMemory(ramsize, vmsize);
        ++aux;
    }
    
    arqMem >> comando;
    if ((comando == "cmem") and (aux == 5)) {
        cache = new Cache(*l1d, *l1i, *l2, *l3);
        Mem = new Memory(*cache, *mp);
        ++aux;
    }

    arqMem.close();

    ifstream instrucoes(nomeArq);

    unsigned short reader;
    //Leitura do arquivo e carregamento para a memória
    int cont = 0;
    while(!instrucoes.eof()){
        instrucoes.read(reinterpret_cast<char*> (&reader), sizeof(unsigned short));
        mp->setMainMemoryData(*mp, cont++, reader);
    }
    instrucoes.close();

    //Execucao dos estagios da maquina
    // agora todas as instruções são executadas, e não
    // quando encontrada a função pop...
    while(cont >= 0){
        cont--;
        BuscaInstrucao();
        Decodificacao();
        //Verificacao do fim do programa e instrucao push/pop
        if((!acabou) and (!pop)){
            ExeMem();
            EscritaRegistrador();
        }
        
        //Verificacao da existencia de Branch
        if(!branch){
            PC += 2;
        }
        //Atualizacao do controle de branch
        branch = false;
    }

    out.close();
    
    cout << "Resultado das operações:" << endl;
    cout << "L1d: " << hitL1d << endl;
    cout << "L1i: " << hitL1i << endl;
    cout << "L2: " << hitL2 << endl;
    cout << "L3: " << hitL3 << endl;
    cout << "LMem: " << hitMemory << endl;
    cout << "erros: " << erros << endl;

    return 0;
}
