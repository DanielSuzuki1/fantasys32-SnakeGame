#pragma once
#include <stdint.h>
#include <vector>
#include <string>

// Segmentação física da memória conforme Seção 1.1 da especificação
const uint32_t TAM_MEMORIA        = 16 * 1024 * 1024; // 16 MB
const uint32_t FRAMEBUFFER_INICIO = 0x00FB4000;       // Início da memória de vídeo
const uint32_t FRAMEBUFFER_FIM    = 0x00FFEFFF;       // Fim da memória de vídeo
const uint32_t PILHA_INICIO       = 0x00FFF000;       // Limite inferior da pilha
const uint32_t PILHA_FIM          = 0x00FFFFFF;       // Inicialização do SP

class Memory {
private:
    std::vector<uint8_t> mem;

public:
    Memory();
    
    // Métodos de acesso à memória de 32 bits (palavra)
    uint32_t ler_palavra(uint32_t endereco);
    void escrever_palavra(uint32_t endereco, uint32_t valor);
    
    // Métodos de conveniência para strings (E/S de texto)
    uint8_t ler_byte(uint32_t endereco);

    // Carrega o binário e extrai o PC inicial (tratando Endianness)
    void carregar_binario(const std::string& caminho_arquivo, uint32_t& pc_inicial);

    // Retorna ponteiro direto para renderização rápida com SDL
    uint8_t* obter_ponteiro_bruto() { return mem.data(); }
};