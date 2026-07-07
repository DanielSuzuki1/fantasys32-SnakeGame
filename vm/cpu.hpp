#pragma once
#include "memory.hpp"
#include <stdint.h>

// Forward Declaration: Avisa ao compilador que a classe Input existe, 
// evitando dependências circulares de cabeçalhos.
class Input;
class Audio; 

class CPU {
private:
    int32_t regs[16];   // 16 registradores de 32 bits (R0 a R15)
    uint32_t seed_lcg;  // Semente do gerador aleatório LCG
    uint64_t frames;    // Contador de frames renderizados
    bool halt;          // Flag de finalização
    bool opt_no_syscall;// Configuração de desativação da SYSCALL

    // Mapeamento explícito de registradores de controle
    const int SP = 14;
    const int PC = 15;

    // Funções auxiliares para manipulação segura da Pilha
    void push(Memory& memoria, uint32_t valor);
    uint32_t pop(Memory& memoria);

public:
    CPU(uint32_t pc_inicial, bool no_syscall);
    
    // Método de execução que recebe a memória e a referência da classe Input
    void executar_instrucao_completa(Memory& memoria, Input& input, Audio& audio);
    
    // Getters e Setters de controle de hardware
    bool deve_parar() const { return halt; }
    void incrementar_frame() { frames++; }
    void imprimir_registradores() const;
};