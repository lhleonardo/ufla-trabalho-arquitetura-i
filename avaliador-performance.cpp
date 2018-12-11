#include <iostream>
#include <cstdio>
#include <memory>
#include <string>
#include <array>
#include <sstream>
#include <chrono>

// para executar um comando externo, capturar seu output e 
// calcular o tempo de execução do mesmo
std::string executarPrograma(const char* comando, long *tempo) {
    std::array<char, 128> buffer;
    std::string result;

    auto tempoInicio = std::chrono::high_resolution_clock::now();
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(comando, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    
    auto tempoFim = std::chrono::high_resolution_clock::now();

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    *tempo = std::chrono::
                duration_cast<std::chrono::microseconds>(tempoFim - tempoInicio)
                .count();

    return result;
}

// para executar o programa: 
// 1o parametro: executável do simulador arm
// 2o parametro: hierarquia de memória 1
// 3o parametro: hierarquia de memoria 2
// 4o parametro: instrucoes em arquivo .o
int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Dados inválidos para execução do programa..." << std::endl;
        std::cerr << std::endl;
        std::cerr << "Para utilizá-lo, os seguintes parâmetros são necesários na execução: "<< std::endl;
        std::cerr << std::endl;
        std::cerr << argv[0] << " executavel hierarquia1 hierarquia2 instrucoes" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Sendo: " << std::endl;
        std::cerr << "\t * executavel = programa que contem o simulador construído" << std::endl;
        std::cerr << "\t * hierarquia1 = arquivo com um estilo de inicialização de" << std::endl;
        std::cerr << "\t\t da memória cache" << std::endl;
        std::cerr << "\t * hierarquia2 = arquivo com um estilo de inicialização de" << std::endl;
        std::cerr << "\t\t da memória cache" << std::endl;
        std::cerr << "\t * instrucoes = compilado (extensão .o) com as instruções" << std::endl;
        std::cerr << "\t\t para um tARM de 16 bits" << std::endl;
        std::cerr << std::endl;
        exit(0);
    }

    // variáveis que representam os arquivos informados por parâmetro
    char *  executavel = argv[1];
    char * hierarquia1 = argv[2];
    char * hierarquia2 = argv[3];
    char * instrucoes  = argv[4];
    
    // definição dos comandos para execução do simulador tARM
    std::stringstream comando1;
    comando1 << "./" << executavel 
             << " " << hierarquia1 
             << " " << instrucoes;

    std::stringstream comando2;
    comando2 << "./" << executavel 
             << " " << hierarquia2 
             << " " << instrucoes;


    // controladores de tempo de execução 
    long tempoPrograma1, tempoPrograma2;

    std::string saida1 = executarPrograma(comando1.str().c_str(), &tempoPrograma1);
    std::string saida2 = executarPrograma(comando2.str().c_str(), &tempoPrograma2);

    std::cout << "-------------------------------" << std::endl;
    std::cout << "Programa 1 (output): " << std::endl;
    std::cout << "-------------------------------" << std::endl;
    std::cout << saida1;
    std::cout << "-------------------------------" << std::endl;
    std::cout << "Tempo de execução: " << tempoPrograma1 << " microssegundos." << std::endl;
    std::cout << "-------------------------------" << std::endl;
    std::cout << std::endl;
    std::cout << "-------------------------------" << std::endl;
    std::cout << "Programa 2 (output): " << std::endl;
    std::cout << "-------------------------------" << std::endl;
    std::cout << saida2;
    std::cout << "-------------------------------" << std::endl;
    std::cout << "Tempo de execução: " << tempoPrograma2 << " microssegundos." << std::endl;
    std::cout << "-------------------------------" << std::endl;
    std::cout << std::endl;
    std::cout << "Fim da análise..." << std::endl;

    return 0;
}