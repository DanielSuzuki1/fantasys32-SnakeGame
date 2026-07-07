#include "memory.hpp"
#include "cpu.hpp"
#include "display.hpp"
#include "input.hpp"
#include "audio.hpp"
#include <iostream>
#include <string>
#include <cstdlib>
#include <SDL2/SDL.h>

void exibir_ajuda() {
    std::cout << "=== Fantasys32 Virtual Machine ===\n"
              << "Uso:\n"
              << "  ./vm [opcoes] caminho/para/o/arquivo.bin\n";
}

int main(int argc, char** argv) {
    std::string caminho_binario = "";
    int escala_janela = 1;
    bool desativar_syscall = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help") {
            exibir_ajuda();
            return EXIT_SUCCESS;
        } else if (arg == "--scale") {
            if (i + 1 < argc) {
                escala_janela = std::atoi(argv[++i]);
            }
        } else if (arg == "--no-syscall") {
            desativar_syscall = true;
        } else {
            caminho_binario = arg;
        }
    }

    if (caminho_binario.empty()) {
        std::cerr << "Erro: Nenhum arquivo binario especificado.\n";
        return EXIT_FAILURE;
    }

    Memory memoria;
    uint32_t pc_inicial = 0;
    memoria.carregar_binario(caminho_binario, pc_inicial);

    // =================================================================
    // 🔄 CORREÇÃO DE ENDIANNESS (Big-Endian para Little-Endian)
    // =================================================================
    // O cabeçalho do arquivo armazena o PC Inicial em Big-Endian.
    // Invertemos a ordem dos bytes para que o sistema nativo leia o valor correto.
    pc_inicial = ((pc_inicial & 0xFF000000) >> 24) |
                 ((pc_inicial & 0x00FF0000) >> 8)  |
                 ((pc_inicial & 0x0000FF00) << 8)  |
                 ((pc_inicial & 0x000000FF) << 24);

    std::cout << "Fantasys32 VM inicializada. Executando binario...\n";

    CPU cpu(pc_inicial, desativar_syscall);
    Display display(escala_janela);
    Input input;
    Audio audio;

    const int FPS = 60;
    const int TEMPO_FRAME_ALVO = 1000 / FPS;

    // ========================================================================================
    // NOTA DE ALTERAÇÃO E AUTORIZAÇÃO (Limites de Hardware)
    // Conforme conversado e autorizado, o limite 
    // original de 10.000 (10^4) instruções por frame foi aumentado para 50.000.
    // O motivo é que o laço de renderização da arena (grama, bordas) somado à renderização
    // de uma cobra muito grande (ex: Score > 650) ultrapassava o teto da CPU, causando um 
    // "slowdown" (câmera lenta) por falta de ciclos de processamento. 
    // Como a taxa de atualização continua perfeitamente travada e sincronizada a 60 FPS,
    // este aumento garante a fluidez do jogo (sem acelerar o tempo) em pontuações extremas.
    // ========================================================================================
    const int INSTRUCOES_POR_FRAME = 50000;

    SDL_Event evento;
    bool forcar_fechamento = false;

    while (!cpu.deve_parar() && !forcar_fechamento) {
        uint64_t inicio_frame = SDL_GetTicks64();

        // Limpa a fila de eventos do sistema operacional para manter a janela ativa
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT) {
                forcar_fechamento = true;
            }
        }

        // Sincroniza os estados brutos do teclado físico do SO com o subsistema da SDL2
        SDL_PumpEvents();

        // Executa o lote de instruções do frame corrente
        for (int i = 0; i < INSTRUCOES_POR_FRAME; i++) {
            cpu.executar_instrucao_completa(memoria, input, audio);
            if (cpu.deve_parar()) break;
        }

        display.renderizar(memoria);
        cpu.incrementar_frame(); 
        audio.atualizar_tempo(TEMPO_FRAME_ALVO);

        uint64_t duracao_frame = SDL_GetTicks64() - inicio_frame;
        if (duracao_frame < (uint64_t)TEMPO_FRAME_ALVO) {
            SDL_Delay(TEMPO_FRAME_ALVO - duracao_frame);
        }
    }

    std::cout << "VM finalizada com sucesso de forma limpa.\n";
    return EXIT_SUCCESS;
}