#include "memory.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>

Memory::Memory() {
    mem.resize(TAM_MEMORIA, 0); // Inicializa os 16MB preenchidos com zero
}

uint8_t Memory::ler_byte(uint32_t endereco) {
    if (endereco >= TAM_MEMORIA) {
        std::cerr << "Erro de Execucao: Acesso a byte fora dos limites (0x" << std::hex << endereco << ")\n";
        std::exit(EXIT_FAILURE);
    }
    return mem[endereco];
}

uint32_t Memory::ler_palavra(uint32_t endereco) {
    // 1. Validação de Limites
    if (endereco >= 0x1000000 || endereco + 3 >= TAM_MEMORIA) {
        std::cerr << "Erro de Execucao: Acesso de leitura fora dos limites (0x" << std::hex << endereco << ")\n";
        std::exit(EXIT_FAILURE);
    }

    // 2. Validação de Alinhamento
    if (endereco % 4 != 0) {
        std::cerr << "Alignment Error: Tentativa de leitura nao alinhada no endereco 0x" << std::hex << endereco << "\n";
        std::exit(EXIT_FAILURE);
    }

    // Lê os bytes invertendo para o formato esperado pelo switch de decodificação da CPU
    return (mem[endereco] << 24)     | 
           (mem[endereco + 1] << 16) | 
           (mem[endereco + 2] << 8)  | 
            mem[endereco + 3];
}

void Memory::escrever_palavra(uint32_t endereco, uint32_t valor) {
    // 1. Validação de Limites
    if (endereco >= 0x1000000 || endereco + 3 >= TAM_MEMORIA) {
        std::cerr << "Erro de Execucao: Acesso de escrita fora dos limites (0x" << std::hex << endereco << ")\n";
        std::exit(EXIT_FAILURE);
    }

    // 2. Validação de Alinhamento
    if (endereco % 4 != 0) {
        std::cerr << "Alignment Error: Tentativa de escrita nao alinhada no endereco 0x" << std::hex << endereco << "\n";
        std::exit(EXIT_FAILURE);
    }

    mem[endereco]     = (valor >> 24) & 0xFF;
    mem[endereco + 1] = (valor >> 16) & 0xFF;
    mem[endereco + 2] = (valor >> 8) & 0xFF;
    mem[endereco + 3] = valor & 0xFF;
}

void Memory::carregar_binario(const std::string& caminho_arquivo, uint32_t& pc_inicial) {
    std::ifstream arquivo(caminho_arquivo, std::ios::binary | std::ios::ate);
    
    if (!arquivo.is_open()) {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo binario: " << caminho_arquivo << "\n";
        std::exit(EXIT_FAILURE);
    }

    std::streamsize tamanho_arquivo = arquivo.tellg();
    if (tamanho_arquivo < 4) {
        std::cerr << "Erro: Arquivo binario invalido ou sem cabecalho.\n";
        std::exit(EXIT_FAILURE);
    }

    arquivo.seekg(0, std::ios::beg);

    // 1. Lê o cabeçalho diretamente (Como ambos são Little-Endian x86, não precisa inverter!)
    uint32_t data_size;
    arquivo.read(reinterpret_cast<char*>(&data_size), sizeof(uint32_t));

    // O PC inicial aponta para onde termina a área de dados e começa o código (.text)
    pc_inicial = data_size;

    // 2. Carrega o resto do código para o início do array de memória
    std::streamsize tamanho_codigo = tamanho_arquivo - 4;
    
    if (tamanho_codigo > PILHA_INICIO) {
        std::cerr << "Erro: O tamanho do codigo excede o limite de memoria geral.\n";
        std::exit(EXIT_FAILURE);
    }

    arquivo.read(reinterpret_cast<char*>(mem.data()), tamanho_codigo);
    arquivo.close();
}